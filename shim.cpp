
#define _GNU_SOURCE
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <android/log.h>

#define LOG_TAG "libgfxshim"
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static pthread_once_t resolver_once = PTHREAD_ONCE_INIT;

typedef void (*ctor_t)(void* /*thisptr*/, unsigned int, unsigned int, int, unsigned int);
typedef void (*dtor_t)(void* /*thisptr*/);

static ctor_t real_ctor = nullptr;
static dtor_t real_dtor = nullptr;

// Common mangled names encountered in 32/64-bit vendor blobs
static const char* mangled_ctor_c1 = "_ZN7android13GraphicBufferC1Ejjij";
static const char* mangled_ctor_c2 = "_ZN7android13GraphicBufferC2Ejjij";
static const char* mangled_dtor_d1 = "_ZN7android13GraphicBufferD1Ev";
static const char* mangled_dtor_d2 = "_ZN7android13GraphicBufferD2Ev";

static const char* candidate_libs[] = {
    "libui.so",
    "libgui.so",
    "libhwui.so",
    "libsurfaceflinger.so",
    "libui_ext.so",
    "libcam_utils.so",
    "libandroid_runtime.so",
    "libbinder.so",
    "libc.so",
    nullptr
};

struct MinimalGraphicBuffer {
    uint64_t magic;
    uint32_t width;
    uint32_t height;
    int32_t  format;
    uint32_t usage;
    void*    reserved[4];
};

// Try resolving symbol by scanning candidate libraries (dlopen/dlsym)
static void try_resolve_from_libs() {
    for (const char** p = candidate_libs; *p; ++p) {
        const char* lib = *p;
        void* handle = dlopen(lib, RTLD_NOW | RTLD_NOLOAD);
        if (!handle) {
            handle = dlopen(lib, RTLD_NOW);
        }
        if (!handle) continue;

        ALOGI("libgfxshim: opened %s", lib);

        void* sym = dlsym(handle, mangled_ctor_c1);
        if (!sym) sym = dlsym(handle, mangled_ctor_c2);
        if (sym) {
            real_ctor = (ctor_t)sym;
            ALOGI("libgfxshim: resolved ctor in %s", lib);
        }

        void* dsym = dlsym(handle, mangled_dtor_d1);
        if (!dsym) dsym = dlsym(handle, mangled_dtor_d2);
        if (dsym) {
            real_dtor = (dtor_t)dsym;
            ALOGI("libgfxshim: resolved dtor in %s", lib);
        }

        if (real_ctor && real_dtor) return;
    }
}

static void init_resolver_once() {
    try_resolve_from_libs();
    // If still not found, attempt to scan /proc/self/maps for libraries and dlopen them
    if (!real_ctor || !real_dtor) {
        FILE* f = fopen("/proc/self/maps", "r");
        if (f) {
            char line[4096];
            while (fgets(line, sizeof(line), f)) {
                // look for ".so" path at end
                char* so = strstr(line, ".so");
                if (!so) continue;
                // find the start of path (first '/')
                char* path = strchr(line, '/');
                if (!path) continue;
                // trim newline and any trailing characters
                char* nl = strchr(path, '\n');
                if (nl) *nl = '\0';
                // try dlopen it
                void* h = dlopen(path, RTLD_NOW | RTLD_NOLOAD);
                if (!h) h = dlopen(path, RTLD_NOW);
                if (!h) continue;
                if (!real_ctor) {
                    void* s = dlsym(h, mangled_ctor_c1);
                    if (!s) s = dlsym(h, mangled_ctor_c2);
                    if (s) {
                        real_ctor = (ctor_t)s;
                        ALOGI("libgfxshim: resolved ctor from %s", path);
                    }
                }
                if (!real_dtor) {
                    void* s = dlsym(h, mangled_dtor_d1);
                    if (!s) s = dlsym(h, mangled_dtor_d2);
                    if (s) {
                        real_dtor = (dtor_t)s;
                        ALOGI("libgfxshim: resolved dtor from %s", path);
                    }
                }
                if (real_ctor && real_dtor) break;
            }
            fclose(f);
        } else {
            ALOGW("libgfxshim: cannot open /proc/self/maps");
        }
    }
}

extern "C" {

// Fallback constructor implementation (mimics original behavior enough for vendor blobs)
void _ZN7android13GraphicBufferC1Ejjij(void* thiz, unsigned int w, unsigned int h, int format, unsigned int usage) {
    pthread_once(&resolver_once, init_resolver_once);
    if (real_ctor) {
        real_ctor(thiz, w, h, format, usage);
        return;
    }

    if (thiz) {
        memset(thiz, 0, sizeof(MinimalGraphicBuffer));
        MinimalGraphicBuffer tmp;
        tmp.magic = 0x47424658ULL; // 'GBFX' or arbitrary magic
        tmp.width = w;
        tmp.height = h;
        tmp.format = format;
        tmp.usage = usage;
        memcpy(thiz, &tmp, sizeof(MinimalGraphicBuffer));
        ALOGW("libgfxshim: used fallback ctor for GraphicBuffer(%u,%u,fmt=%d,usage=%u)", w, h, format, usage);
    }
}

void _ZN7android13GraphicBufferC2Ejjij(void* thiz, unsigned int w, unsigned int h, int format, unsigned int usage) {
    // Typically C2 delegates to C1; do the same
    _ZN7android13GraphicBufferC1Ejjij(thiz, w, h, format, usage);
}

// Provide simple no-op destructors to satisfy vendors that call them
void _ZN7android13GraphicBufferD1Ev(void* thiz) {
    pthread_once(&resolver_once, init_resolver_once);
    if (real_dtor) {
        real_dtor(thiz);
        return;
    }
    // best-effort: clear memory
    if (thiz) {
        memset(thiz, 0, sizeof(MinimalGraphicBuffer));
    }
    ALOGD("libgfxshim: fallback dtor invoked");
}

void _ZN7android13GraphicBufferD2Ev(void* thiz) {
    _ZN7android13GraphicBufferD1Ev(thiz);
}

} // extern "C"

// attempt to pre-resolve when library is loaded
__attribute__((constructor))
static void pre_resolve_shim() {
    pthread_once(&resolver_once, init_resolver_once);
    ALOGI("libgfxshim: constructor finished; real_ctor=%p real_dtor=%p", (void*)real_ctor, (void*)real_dtor);
}
