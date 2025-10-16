#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <string>
#include <atomic>
#include <unistd.h>
#include <pthread.h>
#include <android/log.h>
using uint32 = uint32_t;
using int32 = int32_t;
static const char* TAG = "libshim_gb";
static void mylog(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    __android_log_vprint(ANDROID_LOG_INFO, TAG, fmt, ap);
    va_end(ap);
}
static void* open_lib(const char* path, int mode)
{
    void* h = dlopen(path, mode);
    return h;
}
static void* resolve_libui()
{
    static std::atomic<void*> handle{nullptr};
    void* h = handle.load(std::memory_order_acquire);
    if (h) return h;
    static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&g_lock);
    h = handle.load(std::memory_order_relaxed);
    if (h) { pthread_mutex_unlock(&g_lock); return h; }
    void* cand = nullptr;
    cand = open_lib("/system/lib/libui.so", RTLD_NOW | RTLD_GLOBAL);
    if (!cand) cand = open_lib("/system/lib64/libui.so", RTLD_NOW | RTLD_GLOBAL);
    if (!cand) cand = open_lib("libui.so", RTLD_NOW | RTLD_GLOBAL);
    if (!cand) cand = open_lib("libui.so.0", RTLD_NOW | RTLD_GLOBAL);
    handle.store(cand, std::memory_order_release);
    pthread_mutex_unlock(&g_lock);
    return cand;
}
namespace shim {
using ctor_str_t = void (*)(void*, uint32, uint32, int32, uint32, std::string const&);
using ctor_cstr_t = void (*)(void*, uint32, uint32, int32, uint32, const char*);
using ctor_stride_t = void (*)(void*, uint32, uint32, int32, uint32, uint32, void*, bool);
using dtor_t = void (*)(void*);
struct Resolvers {
    ctor_str_t ctor_str = nullptr;
    ctor_cstr_t ctor_cstr = nullptr;
    ctor_stride_t ctor_stride = nullptr;
    dtor_t dtor = nullptr;
    bool resolved = false;
    void resolve()
    {
        if (resolved) return;
        resolved = true;
        void* h = resolve_libui();
        if (!h) {
            mylog("resolve: libui not found");
            return;
        }
        const char* candidates[] = {
            "_ZN7android13GraphicBufferC1EjjijNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",
            "_ZN7android13GraphicBufferC2EjjijNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",
            "_ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE",
            "_ZN7android13GraphicBufferC2EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE",
            "_ZN7android13GraphicBufferC1EjjijPKc",
            "_ZN7android13GraphicBufferC2EjjijPKc",
            "_ZN7android13GraphicBufferC1EjjijyNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",
            "_ZN7android13GraphicBufferC2EjjijyNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE",
            "_ZN7android13GraphicBufferC1EjjijyNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE",
            "_ZN7android13GraphicBufferC2EjjijyNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE",
            "_ZN7android13GraphicBufferC1EjjijjP11native_handle",
            "_ZN7android13GraphicBufferC2EjjijjP11native_handle",
            "_ZN7android13GraphicBufferC1Ejjij",
            "_ZN7android13GraphicBufferC2Ejjij",
            nullptr
        };
        for (const char** p = candidates; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!ctor_str && strstr(*p,"basic_string")) ctor_str = reinterpret_cast<ctor_str_t>(s);
            if (!ctor_cstr && strstr(*p,"PKc")) ctor_cstr = reinterpret_cast<ctor_cstr_t>(s);
            if (!ctor_stride && strstr(*p,"native_handle")) ctor_stride = reinterpret_cast<ctor_stride_t>(s);
            if (!ctor_str && !ctor_cstr && !ctor_stride) {
                ctor_cstr = reinterpret_cast<ctor_cstr_t>(s);
            }
        }
        void* d = dlsym(h, "_ZN7android13GraphicBufferD1Ev");
        if (!d) d = dlsym(h, "_ZN7android13GraphicBufferD2Ev");
        dtor = reinterpret_cast<dtor_t>(d);
        mylog("resolve: ctor_str=%p ctor_cstr=%p ctor_stride=%p dtor=%p", ctor_str, ctor_cstr, ctor_stride, dtor);
    }
};
static Resolvers g_resolvers;
extern "C" void _ZN7android13GraphicBufferC1Ejjij(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage) asm("_ZN7android13GraphicBufferC1Ejjij");
extern "C" void _ZN7android13GraphicBufferC2Ejjij(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage) asm("_ZN7android13GraphicBufferC2Ejjij");
extern "C" void _ZN7android13GraphicBufferC1Ejjijy(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, unsigned char arg) asm("_ZN7android13GraphicBufferC1Ejjijy");
extern "C" void _ZN7android13GraphicBufferC2Ejjijy(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, unsigned char arg) asm("_ZN7android13GraphicBufferC2Ejjijy");
extern "C" void _ZN7android13GraphicBufferC1EjjijjP11native_handle(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, uint32 inStride, void* inHandle) asm("_ZN7android13GraphicBufferC1EjjijjP11native_handle");
extern "C" void _ZN7android13GraphicBufferC2EjjijjP11native_handle(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, uint32 inStride, void* inHandle) asm("_ZN7android13GraphicBufferC2EjjijjP11native_handle");
extern "C" void _ZN7android13GraphicBufferD1Ev(void* _this) asm("_ZN7android13GraphicBufferD1Ev");
extern "C" void _ZN7android13GraphicBufferD2Ev(void* _this) asm("_ZN7android13GraphicBufferD2Ev");
static void call_ctor_str(void* _this, uint32 w, uint32 h, int32 f, uint32 u, const std::string& s)
{
    if (!g_resolvers.ctor_str) return;
    g_resolvers.ctor_str(_this, w, h, f, u, s);
}
static void call_ctor_cstr(void* _this, uint32 w, uint32 h, int32 f, uint32 u, const char* s)
{
    if (!g_resolvers.ctor_cstr) return;
    g_resolvers.ctor_cstr(_this, w, h, f, u, s);
}
static void call_ctor_stride(void* _this, uint32 w, uint32 h, int32 f, uint32 u, uint32 stride, void* handle)
{
    if (!g_resolvers.ctor_stride) return;
    g_resolvers.ctor_stride(_this, w, h, f, u, stride, handle, false);
}
extern "C" void _ZN7android13GraphicBufferC1Ejjij(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage)
{
    g_resolvers.resolve();
    if (g_resolvers.ctor_str) {
        call_ctor_str(_this, inWidth, inHeight, inFormat, inUsage, std::string(""));
        return;
    }
    if (g_resolvers.ctor_cstr) {
        call_ctor_cstr(_this, inWidth, inHeight, inFormat, inUsage, "");
        return;
    }
    if (g_resolvers.ctor_stride) {
        call_ctor_stride(_this, inWidth, inHeight, inFormat, inUsage, 0, nullptr);
        return;
    }
    memset(_this, 0, sizeof(void*));
    mylog("fallback ctor called for %p", _this);
}
extern "C" void _ZN7android13GraphicBufferC2Ejjij(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage)
{
    g_resolvers.resolve();
    if (g_resolvers.ctor_str) {
        call_ctor_str(_this, inWidth, inHeight, inFormat, inUsage, std::string(""));
        return;
    }
    if (g_resolvers.ctor_cstr) {
        call_ctor_cstr(_this, inWidth, inHeight, inFormat, inUsage, "");
        return;
    }
    if (g_resolvers.ctor_stride) {
        call_ctor_stride(_this, inWidth, inHeight, inFormat, inUsage, 0, nullptr);
        return;
    }
    memset(_this, 0, sizeof(void*));
    mylog("fallback ctor2 called for %p", _this);
}
extern "C" void _ZN7android13GraphicBufferC1Ejjijy(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, unsigned char arg)
{
    g_resolvers.resolve();
    if (g_resolvers.ctor_str) {
        call_ctor_str(_this, inWidth, inHeight, inFormat, inUsage, std::string(""));
        return;
    }
    if (g_resolvers.ctor_cstr) {
        call_ctor_cstr(_this, inWidth, inHeight, inFormat, inUsage, "");
        return;
    }
    memset(_this, 0, sizeof(void*));
    mylog("fallback ctor(y) called for %p", _this);
}
extern "C" void _ZN7android13GraphicBufferC2Ejjijy(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, unsigned char arg)
{
    g_resolvers.resolve();
    if (g_resolvers.ctor_str) {
        call_ctor_str(_this, inWidth, inHeight, inFormat, inUsage, std::string(""));
        return;
    }
    if (g_resolvers.ctor_cstr) {
        call_ctor_cstr(_this, inWidth, inHeight, inFormat, inUsage, "");
        return;
    }
    memset(_this, 0, sizeof(void*));
    mylog("fallback ctor2(y) called for %p", _this);
}
extern "C" void _ZN7android13GraphicBufferC1EjjijjP11native_handle(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, uint32 inStride, void* inHandle)
{
    g_resolvers.resolve();
    if (g_resolvers.ctor_stride) {
        call_ctor_stride(_this, inWidth, inHeight, inFormat, inUsage, inStride, inHandle);
        return;
    }
    if (g_resolvers.ctor_str) {
        call_ctor_str(_this, inWidth, inHeight, inFormat, inUsage, std::string(""));
        return;
    }
    if (g_resolvers.ctor_cstr) {
        call_ctor_cstr(_this, inWidth, inHeight, inFormat, inUsage, "");
        return;
    }
    memset(_this, 0, sizeof(void*));
    mylog("fallback ctor(stride) called for %p", _this);
}
extern "C" void _ZN7android13GraphicBufferC2EjjijjP11native_handle(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, uint32 inStride, void* inHandle)
{
    g_resolvers.resolve();
    if (g_resolvers.ctor_stride) {
        call_ctor_stride(_this, inWidth, inHeight, inFormat, inUsage, inStride, inHandle);
        return;
    }
    if (g_resolvers.ctor_str) {
        call_ctor_str(_this, inWidth, inHeight, inFormat, inUsage, std::string(""));
        return;
    }
    if (g_resolvers.ctor_cstr) {
        call_ctor_cstr(_this, inWidth, inHeight, inFormat, inUsage, "");
        return;
    }
    memset(_this, 0, sizeof(void*));
    mylog("fallback ctor2(stride) called for %p", _this);
}
extern "C" void _ZN7android13GraphicBufferD1Ev(void* _this)
{
    g_resolvers.resolve();
    if (g_resolvers.dtor) {
        g_resolvers.dtor(_this);
        return;
    }
    mylog("dtor fallback for %p", _this);
}
extern "C" void _ZN7android13GraphicBufferD2Ev(void* _this)
{
    g_resolvers.resolve();
    if (g_resolvers.dtor) {
        g_resolvers.dtor(_this);
        return;
    }
    mylog("dtor2 fallback for %p", _this);
}
}
__attribute__((constructor)) static void shim_init(void)
{
    void* h = resolve_libui();
    if (h) {
        mylog("shim_init: libui loaded %p", h);
    } else {
        mylog("shim_init: failed to load libui");
    }
    void* eg = dlopen("libEGL.so", RTLD_NOW | RTLD_GLOBAL);
    void* gles = dlopen("libGLESv2.so", RTLD_NOW | RTLD_GLOBAL);
    mylog("shim_init: libEGL=%p libGLESv2=%p", eg, gles);
}
