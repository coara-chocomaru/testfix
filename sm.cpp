#include <stdint.h>
#include <stddef.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <string>
#include <atomic>
#include <unistd.h>
#include <pthread.h>
using uint32 = uint32_t;
using int32 = int32_t;
static void* open_libui()
{
    static std::atomic<void*> handle{nullptr};
    void* h = handle.load(std::memory_order_acquire);
    if (h) return h;
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    struct pthread_lock_guard { pthread_mutex_t* m; pthread_lock_guard(pthread_mutex_t* mm):m(mm){pthread_mutex_lock(m);} ~pthread_lock_guard(){pthread_mutex_unlock(m);} };
    pthread_lock_guard lk(&m);
    h = handle.load(std::memory_order_relaxed);
    if (h) return h;
    h = dlopen("/system/lib/libui.so", RTLD_LAZY | RTLD_LOCAL);
    if (!h) h = dlopen("/system/lib64/libui.so", RTLD_LAZY | RTLD_LOCAL);
    if (!h) h = dlopen("libui.so", RTLD_LAZY | RTLD_LOCAL);
    if (!h) h = dlopen("libui.so.0", RTLD_LAZY | RTLD_LOCAL);
    handle.store(h, std::memory_order_release);
    return h;
}
namespace shim {
using ctor_new_str_t = void (*)(void*, uint32, uint32, int32, uint32, std::string const&);
using ctor_new_cstr_t = void (*)(void*, uint32, uint32, int32, uint32, const char*);
using ctor_with_stride_t = void (*)(void*, uint32, uint32, int32, uint32, uint32, void*, bool);
using ctor_nativebuf_t = void (*)(void*, void*, bool);
using dtor_t = void (*)(void*);
using initSize_str_t = void (*)(void*, uint32, uint32, int32, uint32, std::string const&);
using initSize_t = void (*)(void*, uint32, uint32, int32, uint32);
using reallocate_t = void (*)(void*, uint32, uint32, int32, uint32);
using free_handle_t = void (*)(void*);
using lock_t = int (*)(void*, uint32, void**);
using lock_rect_t = int (*)(void*, uint32, void*, void**);
using unlock_t = int (*)(void*);
using getNativeBuffer_t = void* (*)(void*);
using getFdCount_t = int (*)(void*);

struct Resolvers {
    ctor_new_str_t new_ctor_str = nullptr;
    ctor_new_cstr_t new_ctor_cstr = nullptr;
    ctor_with_stride_t ctor_stride = nullptr;
    ctor_nativebuf_t ctor_nativebuf = nullptr;
    dtor_t dtor = nullptr;
    initSize_str_t initSize_str = nullptr;
    initSize_t initSize_no_str = nullptr;
    reallocate_t reallocate = nullptr;
    free_handle_t free_handle = nullptr;
    lock_t lock = nullptr;
    lock_rect_t lock_rect = nullptr;
    unlock_t unlock = nullptr;
    getNativeBuffer_t getNativeBuffer = nullptr;
    getFdCount_t getFdCount = nullptr;
    bool resolved = false;
    void resolve() {
        if (resolved) return;
        resolved = true;
        void* h = open_libui();
        if (!h) return;
        const char* candidates_new_str[] = {
            "_ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE",
            "_ZN7android13GraphicBufferC2EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE",
            "_ZN7android13GraphicBufferC1EjjijyNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE",
            "_ZN7android13GraphicBufferC2EjjijyNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE",
            nullptr
        };
        for (const char** p = candidates_new_str; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!new_ctor_str) new_ctor_str = reinterpret_cast<ctor_new_str_t>(s);
        }
        const char* candidates_new_cstr[] = {
            "_ZN7android13GraphicBufferC1EjjijPKc",
            "_ZN7android13GraphicBufferC2EjjijPKc",
            nullptr
        };
        for (const char** p = candidates_new_cstr; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!new_ctor_cstr) new_ctor_cstr = reinterpret_cast<ctor_new_cstr_t>(s);
        }
        const char* candidates_stride[] = {
            "_ZN7android13GraphicBufferC1EjjijjP13native_handle",
            "_ZN7android13GraphicBufferC2EjjijjP13native_handle",
            "_ZN7android13GraphicBufferC1EjjijjP13native_handleb",
            "_ZN7android13GraphicBufferC2EjjijjP13native_handleb",
            nullptr
        };
        for (const char** p = candidates_stride; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!ctor_stride) ctor_stride = reinterpret_cast<ctor_with_stride_t>(s);
        }
        const char* candidates_nativebuf[] = {
            "_ZN7android13GraphicBufferC1EP19ANativeWindowBufferb",
            "_ZN7android13GraphicBufferC2EP19ANativeWindowBufferb",
            nullptr
        };
        for (const char** p = candidates_nativebuf; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!ctor_nativebuf) ctor_nativebuf = reinterpret_cast<ctor_nativebuf_t>(s);
        }
        const char* candidates_dtor[] = {
            "_ZN7android13GraphicBufferD1Ev",
            "_ZN7android13GraphicBufferD2Ev",
            "_ZN7android13GraphicBufferD0Ev",
            nullptr
        };
        for (const char** p = candidates_dtor; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!dtor) dtor = reinterpret_cast<dtor_t>(s);
        }
        const char* candidates_initSize_str[] = {
            "_ZN7android13GraphicBuffer8initSizeEjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE",
            nullptr
        };
        for (const char** p = candidates_initSize_str; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!initSize_str) initSize_str = reinterpret_cast<initSize_str_t>(s);
        }
        const char* candidates_initSize[] = {
            "_ZN7android13GraphicBuffer8initSizeEjjij",
            nullptr
        };
        for (const char** p = candidates_initSize; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!initSize_no_str) initSize_no_str = reinterpret_cast<initSize_t>(s);
        }
        const char* candidates_realloc[] = {
            "_ZN7android13GraphicBuffer10reallocateEjjij",
            nullptr
        };
        for (const char** p = candidates_realloc; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!reallocate) reallocate = reinterpret_cast<reallocate_t>(s);
        }
        const char* candidates_freehandle[] = {
            "_ZN7android13GraphicBuffer11free_handleEv",
            nullptr
        };
        for (const char** p = candidates_freehandle; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!free_handle) free_handle = reinterpret_cast<free_handle_t>(s);
        }
        const char* candidates_lock[] = {
            "_ZN7android13GraphicBuffer4lockEjPPv",
            "_ZN7android13GraphicBuffer9lockAsyncEjPPvi",
            nullptr
        };
        for (const char** p = candidates_lock; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!lock) lock = reinterpret_cast<lock_t>(s);
        }
        const char* candidates_lock_rect[] = {
            "_ZN7android13GraphicBuffer4lockEjRKNS_4RectEPPv",
            "_ZN7android13GraphicBuffer9lockAsyncEjRKNS_4RectEPPvi",
            nullptr
        };
        for (const char** p = candidates_lock_rect; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!lock_rect) lock_rect = reinterpret_cast<lock_rect_t>(s);
        }
        const char* candidates_unlock[] = {
            "_ZN7android13GraphicBuffer6unlockEv",
            nullptr
        };
        for (const char** p = candidates_unlock; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!unlock) unlock = reinterpret_cast<unlock_t>(s);
        }
        const char* candidates_getNativeBuffer[] = {
            "_ZN7android13GraphicBuffer15getNativeBufferEv",
            nullptr
        };
        for (const char** p = candidates_getNativeBuffer; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!getNativeBuffer) getNativeBuffer = reinterpret_cast<getNativeBuffer_t>(s);
        }
        const char* candidates_getFdCount[] = {
            "_ZN7android13GraphicBuffer10getFdCountEv",
            nullptr
        };
        for (const char** p = candidates_getFdCount; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!getFdCount) getFdCount = reinterpret_cast<getFdCount_t>(s);
        }
    }
};
static Resolvers g_resolvers;
extern "C" void _ZN7android13GraphicBufferC1Ejjij(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage) asm("_ZN7android13GraphicBufferC1Ejjij");
extern "C" void _ZN7android13GraphicBufferC2Ejjij(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage) asm("_ZN7android13GraphicBufferC2Ejjij");
extern "C" void _ZN7android13GraphicBufferC1Ejjijy(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, unsigned char arg) asm("_ZN7android13GraphicBufferC1Ejjijy");
extern "C" void _ZN7android13GraphicBufferC2Ejjijy(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, unsigned char arg) asm("_ZN7android13GraphicBufferC2Ejjijy");
extern "C" void _ZN7android13GraphicBufferC1EjjijjP11native_handle(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, uint32 inStride, void* inHandle) asm("_ZN7android13GraphicBufferC1EjjijjP11native_handle");
extern "C" void _ZN7android13GraphicBufferC2EjjijjP11native_handle(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, uint32 inStride, void* inHandle) asm("_ZN7android13GraphicBufferC2EjjijjP11native_handle");
extern "C" void _ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, std::string const& s) asm("_ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE");
extern "C" void _ZN7android13GraphicBufferC2EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, std::string const& s) asm("_ZN7android13GraphicBufferC2EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE");
extern "C" void _ZN7android13GraphicBufferC1EP19ANativeWindowBufferb(void* _this, void* buf, bool b) asm("_ZN7android13GraphicBufferC1EP19ANativeWindowBufferb");
extern "C" void _ZN7android13GraphicBufferC2EP19ANativeWindowBufferb(void* _this, void* buf, bool b) asm("_ZN7android13GraphicBufferC2EP19ANativeWindowBufferb");
extern "C" void _ZN7android13GraphicBufferD1Ev(void* _this) asm("_ZN7android13GraphicBufferD1Ev");
extern "C" void _ZN7android13GraphicBufferD2Ev(void* _this) asm("_ZN7android13GraphicBufferD2Ev");
extern "C" void _ZN7android13GraphicBuffer8initSizeEjjij(void* _this, uint32 w, uint32 h, int32 f, uint32 u) asm("_ZN7android13GraphicBuffer8initSizeEjjij");
extern "C" void _ZN7android13GraphicBuffer8initSizeEjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void* _this, uint32 w, uint32 h, int32 f, uint32 u, std::string const& s) asm("_ZN7android13GraphicBuffer8initSizeEjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE");
extern "C" void _ZN7android13GraphicBuffer10reallocateEjjij(void* _this, uint32 w, uint32 h, int32 f, uint32 u) asm("_ZN7android13GraphicBuffer10reallocateEjjij");
extern "C" void _ZN7android13GraphicBuffer11free_handleEv(void* _this) asm("_ZN7android13GraphicBuffer11free_handleEv");
extern "C" int _ZN7android13GraphicBuffer4lockEjPPv(void* _this, unsigned int usage, void** vaddr) asm("_ZN7android13GraphicBuffer4lockEjPPv");
extern "C" int _ZN7android13GraphicBuffer4lockEjRKNS_4RectEPPv(void* _this, unsigned int usage, void* rect, void** vaddr) asm("_ZN7android13GraphicBuffer4lockEjRKNS_4RectEPPv");
extern "C" int _ZN7android13GraphicBuffer6unlockEv(void* _this) asm("_ZN7android13GraphicBuffer6unlockEv");
extern "C" void* _ZN7android13GraphicBuffer15getNativeBufferEv(void* _this) asm("_ZN7android13GraphicBuffer15getNativeBufferEv");
extern "C" int _ZN7android13GraphicBuffer10getFdCountEv(void* _this) asm("_ZN7android13GraphicBuffer10getFdCountEv");
static const std::string g_empty_string;
static void call_ctor_new_str(void* _this, uint32 w, uint32 h, int32 f, uint32 u, const std::string& s)
{
    if (!g_resolvers.new_ctor_str) return;
    g_resolvers.new_ctor_str(_this, w, h, f, u, s.size()? s : g_empty_string);
}
static void call_ctor_new_cstr(void* _this, uint32 w, uint32 h, int32 f, uint32 u, const char* s)
{
    if (!g_resolvers.new_ctor_cstr) return;
    g_resolvers.new_ctor_cstr(_this, w, h, f, u, s ? s : "");
}
extern "C" void _ZN7android13GraphicBufferC1Ejjij(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage)
{
    g_resolvers.resolve();
    if (g_resolvers.new_ctor_str) {
        call_ctor_new_str(_this, inWidth, inHeight, inFormat, inUsage, std::string(""));
        return;
    }
    if (g_resolvers.new_ctor_cstr) {
        call_ctor_new_cstr(_this, inWidth, inHeight, inFormat, inUsage, "");
        return;
    }
    if (g_resolvers.ctor_nativebuf) {
        g_resolvers.ctor_nativebuf(_this, nullptr, false);
        return;
    }
    std::memset(_this, 0, sizeof(void*));
}
extern "C" void _ZN7android13GraphicBufferC2Ejjij(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage)
{
    g_resolvers.resolve();
    if (g_resolvers.new_ctor_str) {
        call_ctor_new_str(_this, inWidth, inHeight, inFormat, inUsage, std::string(""));
        return;
    }
    if (g_resolvers.new_ctor_cstr) {
        call_ctor_new_cstr(_this, inWidth, inHeight, inFormat, inUsage, "");
        return;
    }
    if (g_resolvers.ctor_nativebuf) {
        g_resolvers.ctor_nativebuf(_this, nullptr, false);
        return;
    }
    std::memset(_this, 0, sizeof(void*));
}
extern "C" void _ZN7android13GraphicBufferC1Ejjijy(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, unsigned char arg)
{
    g_resolvers.resolve();
    if (g_resolvers.new_ctor_str) {
        call_ctor_new_str(_this, inWidth, inHeight, inFormat, inUsage, std::string(""));
        return;
    }
    if (g_resolvers.new_ctor_cstr) {
        call_ctor_new_cstr(_this, inWidth, inHeight, inFormat, inUsage, "");
        return;
    }
    if (g_resolvers.ctor_nativebuf) {
        g_resolvers.ctor_nativebuf(_this, nullptr, false);
        return;
    }
    std::memset(_this, 0, sizeof(void*));
}
extern "C" void _ZN7android13GraphicBufferC2Ejjijy(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, unsigned char arg)
{
    g_resolvers.resolve();
    if (g_resolvers.new_ctor_str) {
        call_ctor_new_str(_this, inWidth, inHeight, inFormat, inUsage, std::string(""));
        return;
    }
    if (g_resolvers.new_ctor_cstr) {
        call_ctor_new_cstr(_this, inWidth, inHeight, inFormat, inUsage, "");
        return;
    }
    if (g_resolvers.ctor_nativebuf) {
        g_resolvers.ctor_nativebuf(_this, nullptr, false);
        return;
    }
    std::memset(_this, 0, sizeof(void*));
}
extern "C" void _ZN7android13GraphicBufferC1EjjijjP11native_handle(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, uint32 inStride, void* inHandle)
{
    g_resolvers.resolve();
    if (g_resolvers.ctor_stride) {
        g_resolvers.ctor_stride(_this, inWidth, inHeight, inFormat, inUsage, inStride, inHandle, false);
        return;
    }
    if (g_resolvers.new_ctor_str) {
        call_ctor_new_str(_this, inWidth, inHeight, inFormat, inUsage, std::string(""));
        return;
    }
    std::memset(_this, 0, sizeof(void*));
}
extern "C" void _ZN7android13GraphicBufferC2EjjijjP11native_handle(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, uint32 inStride, void* inHandle)
{
    g_resolvers.resolve();
    if (g_resolvers.ctor_stride) {
        g_resolvers.ctor_stride(_this, inWidth, inHeight, inFormat, inUsage, inStride, inHandle, false);
        return;
    }
    if (g_resolvers.new_ctor_str) {
        call_ctor_new_str(_this, inWidth, inHeight, inFormat, inUsage, std::string(""));
        return;
    }
    std::memset(_this, 0, sizeof(void*));
}
extern "C" void _ZN7android13GraphicBufferC1EP19ANativeWindowBufferb(void* _this, void* buf, bool b)
{
    g_resolvers.resolve();
    if (g_resolvers.ctor_nativebuf) {
        g_resolvers.ctor_nativebuf(_this, buf, b);
        return;
    }
    std::memset(_this, 0, sizeof(void*));
}
extern "C" void _ZN7android13GraphicBufferC2EP19ANativeWindowBufferb(void* _this, void* buf, bool b)
{
    g_resolvers.resolve();
    if (g_resolvers.ctor_nativebuf) {
        g_resolvers.ctor_nativebuf(_this, buf, b);
        return;
    }
    std::memset(_this, 0, sizeof(void*));
}
extern "C" void _ZN7android13GraphicBufferD1Ev(void* _this)
{
    g_resolvers.resolve();
    if (g_resolvers.dtor) {
        g_resolvers.dtor(_this);
        return;
    }
}
extern "C" void _ZN7android13GraphicBufferD2Ev(void* _this)
{
    g_resolvers.resolve();
    if (g_resolvers.dtor) {
        g_resolvers.dtor(_this);
        return;
    }
}
extern "C" void _ZN7android13GraphicBuffer8initSizeEjjij(void* _this, uint32 w, uint32 h, int32 f, uint32 u)
{
    g_resolvers.resolve();
    if (g_resolvers.initSize_no_str) {
        g_resolvers.initSize_no_str(_this, w, h, f, u);
        return;
    }
    if (g_resolvers.initSize_str) {
        g_resolvers.initSize_str(_this, w, h, f, u, std::string(""));
        return;
    }
}
extern "C" void _ZN7android13GraphicBuffer8initSizeEjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void* _this, uint32 w, uint32 h, int32 f, uint32 u, std::string const& s)
{
    g_resolvers.resolve();
    if (g_resolvers.initSize_str) {
        g_resolvers.initSize_str(_this, w, h, f, u, s);
        return;
    }
    if (g_resolvers.initSize_no_str) {
        g_resolvers.initSize_no_str(_this, w, h, f, u);
        return;
    }
}
extern "C" void _ZN7android13GraphicBuffer10reallocateEjjij(void* _this, uint32 w, uint32 h, int32 f, uint32 u)
{
    g_resolvers.resolve();
    if (g_resolvers.reallocate) {
        g_resolvers.reallocate(_this, w, h, f, u);
        return;
    }
}
extern "C" void _ZN7android13GraphicBuffer11free_handleEv(void* _this)
{
    g_resolvers.resolve();
    if (g_resolvers.free_handle) {
        g_resolvers.free_handle(_this);
        return;
    }
}
extern "C" int _ZN7android13GraphicBuffer4lockEjPPv(void* _this, unsigned int usage, void** vaddr)
{
    g_resolvers.resolve();
    if (g_resolvers.lock) {
        return g_resolvers.lock(_this, usage, vaddr);
    }
    return -1;
}
extern "C" int _ZN7android13GraphicBuffer4lockEjRKNS_4RectEPPv(void* _this, unsigned int usage, void* rect, void** vaddr)
{
    g_resolvers.resolve();
    if (g_resolvers.lock_rect) {
        return g_resolvers.lock_rect(_this, usage, rect, vaddr);
    }
    if (g_resolvers.lock) {
        return g_resolvers.lock(_this, usage, vaddr);
    }
    return -1;
}
extern "C" int _ZN7android13GraphicBuffer6unlockEv(void* _this)
{
    g_resolvers.resolve();
    if (g_resolvers.unlock) {
        return g_resolvers.unlock(_this);
    }
    return -1;
}
extern "C" void* _ZN7android13GraphicBuffer15getNativeBufferEv(void* _this)
{
    g_resolvers.resolve();
    if (g_resolvers.getNativeBuffer) {
        return g_resolvers.getNativeBuffer(_this);
    }
    return nullptr;
}
extern "C" int _ZN7android13GraphicBuffer10getFdCountEv(void* _this)
{
    g_resolvers.resolve();
    if (g_resolvers.getFdCount) {
        return g_resolvers.getFdCount(_this);
    }
    return 0;
}
}
