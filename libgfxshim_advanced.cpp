#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <android/log.h>
#include <dirent.h>
#include <link.h>
#include <inttypes.h>

#define LOG_TAG "libgfxshim"
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static pthread_once_t resolver_once = PTHREAD_ONCE_INIT;

typedef void (*ctor_any_t)(void*, unsigned long long, unsigned long long, int, unsigned long long);
typedef void (*dtor_any_t)(void*);

static ctor_any_t real_ctor = nullptr;
static dtor_any_t real_dtor = nullptr;

static const char* initial_candidate_libs[] = {
    "libui.so",
    "libgui.so",
    "libui_ext.so",
    "libsurfaceflinger.so",
    "libhwui.so",
    "libhwcomposer.so",
    "libgui_ext.so",
    "libbinder.so",
    "libandroid_runtime.so",
    "libc.so",
    nullptr
};

static const char* lib_search_dirs[] = {
    "/system/lib64",
    "/system/lib",
    "/vendor/lib64",
    "/vendor/lib",
    "/product/lib64",
    "/product/lib",
    nullptr
};

struct MinimalGraphicBuffer {
    uint64_t magic;
    uint64_t width;
    uint64_t height;
    int32_t  format;
    uint64_t usage;
    void*    reserved[6];
};

static void try_assign_ctor_dtor(void* sym, const char* name) {
    if (!sym) return;
    if (!real_ctor && (strstr(name, "GraphicBufferC1") || strstr(name, "GraphicBufferC2") || strstr(name, "GraphicBufferC"))) {
        real_ctor = (ctor_any_t)sym;
        ALOGI("libgfxshim: assigned ctor -> %s (%p)", name, sym);
    }
    if (!real_dtor && (strstr(name, "GraphicBufferD1") || strstr(name, "GraphicBufferD2") || strstr(name, "GraphicBufferD"))) {
        real_dtor = (dtor_any_t)sym;
        ALOGI("libgfxshim: assigned dtor -> %s (%p)", name, sym);
    }
}

static bool try_dlsym_with_handle(void* handle, const char* mangled) {
    if (!mangled) return false;
    void* s = nullptr;
    if (handle)
        s = dlsym(handle, mangled);
    else
        s = dlsym(RTLD_DEFAULT, mangled);
    if (s) {
        try_assign_ctor_dtor(s, mangled);
        return true;
    }
    return false;
}

static void try_symbol_variations_on_handle(void* handle, const char* basePrefix, const char* suffix) {
    const char* typeVariants[] = {
        "jjij","mmij","jjii","jji","jjib","jjim","Emmij","Ejjij","Ejjii","Ejji","Ejji","Emmii","", nullptr
    };
    for (const char** tv = typeVariants; *tv; ++tv) {
        char mangled[256];
        snprintf(mangled, sizeof(mangled), "%s%s%s", basePrefix, *tv, suffix ? suffix : "");
        try_dlsym_with_handle(handle, mangled);
        if (real_ctor && real_dtor) return;
    }
}

static void try_common_mangled_set(void* handle) {
    const char* bases[] = {
        "_ZN7android13GraphicBufferC1E",
        "_ZN7android13GraphicBufferC2E",
        "_ZN7android13GraphicBufferC1Em",
        "_ZN7android13GraphicBufferC2Em",
        "_ZN7android13GraphicBufferC1Ej",
        "_ZN7android13GraphicBufferC2Ej",
        "_ZN7android13GraphicBufferC1E",
        "_ZN7android13GraphicBufferC2E",
        "_ZN7android13GraphicBufferD1Ev",
        "_ZN7android13GraphicBufferD2Ev",
        nullptr
    };
    for (const char** b = bases; *b; ++b) {
        if (strstr(*b, "D1Ev") || strstr(*b, "D2Ev")) {
            try_dlsym_with_handle(handle, *b);
        } else {
            try_symbol_variations_on_handle(handle, *b, nullptr);
        }
        if (real_ctor && real_dtor) return;
    }
}

static int phdr_callback(struct dl_phdr_info *info, size_t size, void *data) {
    if (!info || !info->dlpi_name || !info->dlpi_name[0]) return 0;
    void* h = dlopen(info->dlpi_name, RTLD_NOW | RTLD_NOLOAD);
    if (!h) h = dlopen(info->dlpi_name, RTLD_NOW);
    if (!h) return 0;
    try_common_mangled_set(h);
    dlclose(h);
    if (real_ctor && real_dtor) return 1;
    return 0;
}

static void scan_candidate_libs() {
    for (const char** p = initial_candidate_libs; *p; ++p) {
        void* h = dlopen(*p, RTLD_NOW | RTLD_NOLOAD);
        if (!h) h = dlopen(*p, RTLD_NOW);
        if (!h) continue;
        try_common_mangled_set(h);
        dlclose(h);
        if (real_ctor && real_dtor) return;
    }
}

static void scan_proc_maps_and_try() {
    FILE* f = fopen("/proc/self/maps", "r");
    if (!f) return;
    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        char* path = strchr(line, '/');
        if (!path) continue;
        char* nl = strchr(path, '\n');
        if (nl) *nl = '\0';
        void* h = dlopen(path, RTLD_NOW | RTLD_NOLOAD);
        if (!h) h = dlopen(path, RTLD_NOW);
        if (!h) continue;
        try_common_mangled_set(h);
        dlclose(h);
        if (real_ctor && real_dtor) break;
    }
    fclose(f);
}

static void scan_library_dirs() {
    for (const char** d = lib_search_dirs; *d; ++d) {
        DIR* dir = opendir(*d);
        if (!dir) continue;
        struct dirent* ent;
        while ((ent = readdir(dir)) != nullptr) {
            if (ent->d_type != DT_REG && ent->d_type != DT_LNK && ent->d_type != DT_UNKNOWN) continue;
            if (!strstr(ent->d_name, ".so")) continue;
            char pathbuf[512];
            snprintf(pathbuf, sizeof(pathbuf), "%s/%s", *d, ent->d_name);
            void* h = dlopen(pathbuf, RTLD_NOW | RTLD_NOLOAD);
            if (!h) h = dlopen(pathbuf, RTLD_NOW);
            if (!h) continue;
            try_common_mangled_set(h);
            dlclose(h);
            if (real_ctor && real_dtor) break;
        }
        closedir(dir);
        if (real_ctor && real_dtor) break;
    }
}

static void init_resolver_once() {
    try_common_mangled_set(nullptr);
    if (real_ctor && real_dtor) return;
    scan_candidate_libs();
    if (real_ctor && real_dtor) return;
    scan_proc_maps_and_try();
    if (real_ctor && real_dtor) return;
    dl_iterate_phdr(phdr_callback, nullptr);
    if (real_ctor && real_dtor) return;
    scan_library_dirs();
    if (!real_ctor || !real_dtor) {
        ALOGW("libgfxshim: resolver incomplete real_ctor=%p real_dtor=%p", (void*)real_ctor, (void*)real_dtor);
    } else {
        ALOGI("libgfxshim: resolved ctor %p and dtor %p", (void*)real_ctor, (void*)real_dtor);
    }
}

static void gfx_ctor_common(void* thiz, unsigned long long w, unsigned long long h, int format, unsigned long long usage) {
    pthread_once(&resolver_once, init_resolver_once);
    if (real_ctor) {
        real_ctor(thiz, w, h, format, usage);
        return;
    }
    if (thiz) {
        MinimalGraphicBuffer tmp;
        memset(&tmp, 0, sizeof(tmp));
        tmp.magic = 0x47424658ULL;
        tmp.width = w;
        tmp.height = h;
        tmp.format = format;
        tmp.usage = usage;
        memcpy(thiz, &tmp, (sizeof(tmp) < sizeof(MinimalGraphicBuffer) ? sizeof(tmp) : sizeof(MinimalGraphicBuffer)));
        ALOGW("libgfxshim: fallback ctor used w=%" PRIu64 " h=%" PRIu64 " fmt=%d usage=%" PRIu64, (uint64_t)w, (uint64_t)h, format, (uint64_t)usage);
    }
}

static void gfx_dtor_common(void* thiz) {
    pthread_once(&resolver_once, init_resolver_once);
    if (real_dtor) {
        real_dtor(thiz);
        return;
    }
    if (thiz) {
        memset(thiz, 0, sizeof(MinimalGraphicBuffer));
        ALOGD("libgfxshim: fallback dtor invoked");
    }
}

extern "C" {

void _ZN7android13GraphicBufferC1Ejjij(void* thiz, unsigned int w, unsigned int h, int format, unsigned int usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }
void _ZN7android13GraphicBufferC2Ejjij(void* thiz, unsigned int w, unsigned int h, int format, unsigned int usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }

void _ZN7android13GraphicBufferC1Emmij(void* thiz, unsigned long w, unsigned long h, int format, unsigned long usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }
void _ZN7android13GraphicBufferC2Emmij(void* thiz, unsigned long w, unsigned long h, int format, unsigned long usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }

void _ZN7android13GraphicBufferC1Ejjii(void* thiz, unsigned int w, unsigned int h, int format, int usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }
void _ZN7android13GraphicBufferC2Ejjii(void* thiz, unsigned int w, unsigned int h, int format, int usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }

void _ZN7android13GraphicBufferC1Ejji(void* thiz, unsigned int w, unsigned int h, int usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, 0, (unsigned long long)usage); }
void _ZN7android13GraphicBufferC2Ejji(void* thiz, unsigned int w, unsigned int h, int usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, 0, (unsigned long long)usage); }

void _ZN7android13GraphicBufferC1Ejjib(void* thiz, unsigned int w, unsigned int h, int format, unsigned char usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }
void _ZN7android13GraphicBufferC2Ejjib(void* thiz, unsigned int w, unsigned int h, int format, unsigned char usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }

void _ZN7android13GraphicBufferC1Ejjim(void* thiz, unsigned int w, unsigned int h, int format, unsigned long usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }
void _ZN7android13GraphicBufferC2Ejjim(void* thiz, unsigned int w, unsigned int h, int format, unsigned long usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }

void _ZN7android13GraphicBufferD1Ev(void* thiz) { gfx_dtor_common(thiz); }
void _ZN7android13GraphicBufferD2Ev(void* thiz) { gfx_dtor_common(thiz); }

}

__attribute__((constructor))
static void pre_resolve_shim() {
    pthread_once(&resolver_once, init_resolver_once);
    ALOGI("libgfxshim: constructor finished; real_ctor=%p real_dtor=%p", (void*)real_ctor, (void*)real_dtor);
}
