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
typedef int  (*lock_any_t)(void*, unsigned int, void**);
typedef int  (*unlock_any_t)(void*);
typedef unsigned int (*get_uint_any_t)(void*);
typedef void* (*get_native_any_t)(void*);

static ctor_any_t real_ctor = nullptr;
static dtor_any_t real_dtor = nullptr;
static lock_any_t real_lock = nullptr;
static unlock_any_t real_unlock = nullptr;
static get_uint_any_t real_getWidth = nullptr;
static get_uint_any_t real_getHeight = nullptr;
static get_uint_any_t real_getUsage = nullptr;
static get_uint_any_t real_getPixelFormat = nullptr;
static get_native_any_t real_getNativeBuffer = nullptr;

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
    uint32_t reserved32[4];
    void* reserved[4];
};

static void assign_if_ctor_dtor(void* sym, const char* name) {
    if (!sym) return;
    if (!real_ctor && (strstr(name, "GraphicBufferC1") || strstr(name, "GraphicBufferC2") || strstr(name, "GraphicBufferC"))) {
        real_ctor = (ctor_any_t)sym;
        ALOGI("libgfxshim: resolved ctor %s -> %p", name, sym);
    }
    if (!real_dtor && (strstr(name, "GraphicBufferD1") || strstr(name, "GraphicBufferD2") || strstr(name, "GraphicBufferD"))) {
        real_dtor = (dtor_any_t)sym;
        ALOGI("libgfxshim: resolved dtor %s -> %p", name, sym);
    }
}

static void assign_method_if_matches(void* sym, const char* name) {
    if (!sym) return;
    if (!real_lock && (strstr(name, "GraphicBuffer4lock") || strstr(name, "GraphicBuffer4lockEj") || strstr(name, "GraphicBuffer4lockEjPPv"))) {
        real_lock = (lock_any_t)sym;
        ALOGI("libgfxshim: resolved lock %s -> %p", name, sym);
    }
    if (!real_unlock && (strstr(name, "GraphicBuffer6unlock") || strstr(name, "GraphicBuffer6unlockEv"))) {
        real_unlock = (unlock_any_t)sym;
        ALOGI("libgfxshim: resolved unlock %s -> %p", name, sym);
    }
    if (!real_getWidth && strstr(name, "GraphicBuffer8getWidth")) {
        real_getWidth = (get_uint_any_t)sym;
        ALOGI("libgfxshim: resolved getWidth %s -> %p", name, sym);
    }
    if (!real_getHeight && strstr(name, "GraphicBuffer9getHeight")) {
        real_getHeight = (get_uint_any_t)sym;
        ALOGI("libgfxshim: resolved getHeight %s -> %p", name, sym);
    }
    if (!real_getUsage && (strstr(name, "GraphicBuffer8getUsage") || strstr(name, "GraphicBuffer8getUsageEv"))) {
        real_getUsage = (get_uint_any_t)sym;
        ALOGI("libgfxshim: resolved getUsage %s -> %p", name, sym);
    }
    if (!real_getPixelFormat && (strstr(name, "GraphicBuffer10getPixel") || strstr(name, "getPixelFormat"))) {
        real_getPixelFormat = (get_uint_any_t)sym;
        ALOGI("libgfxshim: resolved getPixelFormat %s -> %p", name, sym);
    }
    if (!real_getNativeBuffer && (strstr(name, "getNativeBuffer") || strstr(name, "getNativeBufferEv"))) {
        real_getNativeBuffer = (get_native_any_t)sym;
        ALOGI("libgfxshim: resolved getNativeBuffer %s -> %p", name, sym);
    }
}

static bool try_dlsym_handle(void* handle, const char* symname) {
    if (!symname) return false;
    void* s = nullptr;
    if (handle)
        s = dlsym(handle, symname);
    else
        s = dlsym(RTLD_DEFAULT, symname);
    if (s) {
        assign_if_ctor_dtor(s, symname);
        assign_method_if_matches(s, symname);
        return true;
    }
    return false;
}

static void try_symbol_variants(void* handle) {
    const char* ctorBases[] = {
        "_ZN7android13GraphicBufferC1E", "_ZN7android13GraphicBufferC2E",
        "_ZN7android13GraphicBufferC1Em", "_ZN7android13GraphicBufferC2Em",
        "_ZN7android13GraphicBufferC1Ej", "_ZN7android13GraphicBufferC2Ej",
        nullptr
    };
    const char* argTails[] = {"jjij","mmij","jjii","jji","jjib","jjim","", nullptr};
    for (const char** b = ctorBases; *b; ++b) {
        for (const char** t = argTails; *t; ++t) {
            char mangled[256];
            snprintf(mangled, sizeof(mangled), "%s%s", *b, *t);
            try_dlsym_handle(handle, mangled);
            if (real_ctor && real_dtor) return;
        }
    }
    const char* dtorBases[] = {"_ZN7android13GraphicBufferD1Ev","_ZN7android13GraphicBufferD2Ev", nullptr};
    for (const char** d = dtorBases; *d; ++d) {
        try_dlsym_handle(handle, *d);
        if (real_ctor && real_dtor) return;
    }
    const char* meths[] = {
        "_ZN7android13GraphicBuffer4lockEjPPv",
        "_ZN7android13GraphicBuffer4lockEjPv",
        "_ZN7android13GraphicBuffer4lockEj",
        "_ZN7android13GraphicBuffer6unlockEv",
        "_ZNK7android13GraphicBuffer8getWidthEv",
        "_ZNK7android13GraphicBuffer9getHeightEv",
        "_ZNK7android13GraphicBuffer8getUsageEv",
        "_ZNK7android13GraphicBuffer10getPixelFormatEv",
        "_ZNK7android13GraphicBuffer15getNativeBufferEv",
        "_ZN7android13GraphicBuffer4lockEjPv",
        nullptr
    };
    for (const char** m = meths; *m; ++m) {
        try_dlsym_handle(handle, *m);
        if (real_lock && real_unlock && real_getWidth && real_getHeight && real_getUsage && real_getPixelFormat && real_getNativeBuffer) break;
    }
}

static void scan_candidate_libs() {
    for (const char** p = initial_candidate_libs; *p; ++p) {
        void* h = dlopen(*p, RTLD_NOW | RTLD_NOLOAD);
        if (!h) h = dlopen(*p, RTLD_NOW);
        if (!h) continue;
        try_symbol_variants(h);
        dlclose(h);
        if (real_ctor && real_dtor && real_lock && real_unlock) return;
    }
}

static void scan_proc_maps() {
    FILE* f = fopen("/proc/self/maps","r");
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
        try_symbol_variants(h);
        dlclose(h);
        if (real_ctor && real_dtor) break;
    }
    fclose(f);
}

static int phdr_cb(struct dl_phdr_info *info, size_t size, void *data) {
    if (!info || !info->dlpi_name || !info->dlpi_name[0]) return 0;
    void* h = dlopen(info->dlpi_name, RTLD_NOW | RTLD_NOLOAD);
    if (!h) h = dlopen(info->dlpi_name, RTLD_NOW);
    if (!h) return 0;
    try_symbol_variants(h);
    dlclose(h);
    if (real_ctor && real_dtor) return 1;
    return 0;
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
            try_symbol_variants(h);
            dlclose(h);
            if (real_ctor && real_dtor) break;
        }
        closedir(dir);
        if (real_ctor && real_dtor) break;
    }
}

static void init_resolver_once() {
    try_symbol_variants(nullptr);
    if (real_ctor && real_dtor) return;
    scan_candidate_libs();
    if (real_ctor && real_dtor) return;
    scan_proc_maps();
    if (real_ctor && real_dtor) return;
    dl_iterate_phdr(phdr_cb, nullptr);
    if (real_ctor && real_dtor) return;
    scan_library_dirs();
    if (!real_ctor || !real_dtor) {
        ALOGW("libgfxshim: resolver incomplete ctor=%p dtor=%p lock=%p unlock=%p", (void*)real_ctor, (void*)real_dtor, (void*)real_lock, (void*)real_unlock);
    } else {
        ALOGI("libgfxshim: resolver done ctor=%p dtor=%p", (void*)real_ctor, (void*)real_dtor);
    }
}

static void fallback_init_object(void* thiz, uint64_t w, uint64_t h, int fmt, uint64_t usage) {
    if (!thiz) return;
    MinimalGraphicBuffer tmp;
    memset(&tmp, 0, sizeof(tmp));
    tmp.magic = 0x47424658ULL;
    tmp.width = w;
    tmp.height = h;
    tmp.format = fmt;
    tmp.usage = usage;
    memcpy(thiz, &tmp, sizeof(tmp));
    ALOGW("libgfxshim: fallback object init w=%" PRIu64 " h=%" PRIu64 " fmt=%d usage=%" PRIu64, w, h, fmt, usage);
}

static void gfx_ctor_common(void* thiz, unsigned long long w, unsigned long long h, int format, unsigned long long usage) {
    pthread_once(&resolver_once, init_resolver_once);
    if (real_ctor) {
        real_ctor(thiz, w, h, format, usage);
        return;
    }
    fallback_init_object(thiz, w, h, format, usage);
}

static void gfx_dtor_common(void* thiz) {
    pthread_once(&resolver_once, init_resolver_once);
    if (real_dtor) {
        real_dtor(thiz);
        return;
    }
    if (thiz) memset(thiz, 0, sizeof(MinimalGraphicBuffer));
    ALOGD("libgfxshim: fallback dtor");
}

static int gfx_lock_common(void* thiz, unsigned int usage, void** vaddr) {
    pthread_once(&resolver_once, init_resolver_once);
    if (real_lock) return real_lock(thiz, usage, vaddr);
    if (vaddr) *vaddr = nullptr;
    return 0;
}

static int gfx_unlock_common(void* thiz) {
    pthread_once(&resolver_once, init_resolver_once);
    if (real_unlock) return real_unlock(thiz);
    return 0;
}

static unsigned int gfx_getWidth_common(void* thiz) {
    pthread_once(&resolver_once, init_resolver_once);
    if (real_getWidth) return real_getWidth(thiz);
    if (!thiz) return 0;
    MinimalGraphicBuffer tmp;
    memcpy(&tmp, thiz, sizeof(MinimalGraphicBuffer));
    return (unsigned int)tmp.width;
}

static unsigned int gfx_getHeight_common(void* thiz) {
    pthread_once(&resolver_once, init_resolver_once);
    if (real_getHeight) return real_getHeight(thiz);
    if (!thiz) return 0;
    MinimalGraphicBuffer tmp;
    memcpy(&tmp, thiz, sizeof(MinimalGraphicBuffer));
    return (unsigned int)tmp.height;
}

static unsigned int gfx_getUsage_common(void* thiz) {
    pthread_once(&resolver_once, init_resolver_once);
    if (real_getUsage) return real_getUsage(thiz);
    if (!thiz) return 0;
    MinimalGraphicBuffer tmp;
    memcpy(&tmp, thiz, sizeof(MinimalGraphicBuffer));
    return (unsigned int)tmp.usage;
}

static unsigned int gfx_getPixelFormat_common(void* thiz) {
    pthread_once(&resolver_once, init_resolver_once);
    if (real_getPixelFormat) return real_getPixelFormat(thiz);
    if (!thiz) return 0;
    MinimalGraphicBuffer tmp;
    memcpy(&tmp, thiz, sizeof(MinimalGraphicBuffer));
    return (unsigned int)tmp.format;
}

static void* gfx_getNativeBuffer_common(void* thiz) {
    pthread_once(&resolver_once, init_resolver_once);
    if (real_getNativeBuffer) return real_getNativeBuffer(thiz);
    return nullptr;
}

extern "C" {

void _ZN7android13GraphicBufferC1Ejjij(void* thiz, unsigned int w, unsigned int h, int format, unsigned int usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }
void _ZN7android13GraphicBufferC2Ejjij(void* thiz, unsigned int w, unsigned int h, int format, unsigned int usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }
void _ZN7android13GraphicBufferC1Emmij(void* thiz, unsigned long w, unsigned long h, int format, unsigned long usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }
void _ZN7android13GraphicBufferC2Emmij(void* thiz, unsigned long w, unsigned long h, int format, unsigned long usage) { gfx_ctor_common(thiz, (unsigned long long)w, (unsigned long long)h, format, (unsigned long long)usage); }
void _ZN7android13GraphicBufferD1Ev(void* thiz) { gfx_dtor_common(thiz); }
void _ZN7android13GraphicBufferD2Ev(void* thiz) { gfx_dtor_common(thiz); }

int _ZN7android13GraphicBuffer4lockEjPPv(void* thiz, unsigned int usage, void** vaddr) { return gfx_lock_common(thiz, usage, vaddr); }
int _ZN7android13GraphicBuffer4lockEjPv(void* thiz, unsigned int usage, void* ptr) { void** p = nullptr; return gfx_lock_common(thiz, usage, p); }
int _ZN7android13GraphicBuffer4lockEj(void* thiz, unsigned int usage) { return gfx_lock_common(thiz, usage, nullptr); }
int _ZN7android13GraphicBuffer6unlockEv(void* thiz) { return gfx_unlock_common(thiz); }

unsigned int _ZNK7android13GraphicBuffer8getWidthEv(void* thiz) { return gfx_getWidth_common(thiz); }
unsigned int _ZNK7android13GraphicBuffer9getHeightEv(void* thiz) { return gfx_getHeight_common(thiz); }
unsigned int _ZNK7android13GraphicBuffer8getUsageEv(void* thiz) { return gfx_getUsage_common(thiz); }
unsigned int _ZNK7android13GraphicBuffer10getPixelFormatEv(void* thiz) { return gfx_getPixelFormat_common(thiz); }
void* _ZNK7android13GraphicBuffer15getNativeBufferEv(void* thiz) { return gfx_getNativeBuffer_common(thiz); }

}

__attribute__((constructor))
static void pre_resolve_shim() {
    pthread_once(&resolver_once, init_resolver_once);
    ALOGI("libgfxshim: constructor complete ctor=%p dtor=%p lock=%p unlock=%p", (void*)real_ctor, (void*)real_dtor, (void*)real_lock, (void*)real_unlock);
}
