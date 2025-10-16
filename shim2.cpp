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
using dtor_t = void (*)(void*);
struct Resolvers {
    ctor_new_str_t new_ctor_str = nullptr;
    ctor_new_cstr_t new_ctor_cstr = nullptr;
    ctor_with_stride_t ctor_stride = nullptr;
    dtor_t dtor = nullptr;
    bool resolved = false;
    void resolve() {
        if (resolved) return;
        resolved = true;
        void* h = open_libui();
        if (!h) return;
        const char* candidates_new[] = {
            "_ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE",
            "_ZN7android13GraphicBufferC2EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE",
            "_ZN7android13GraphicBufferC1EjjijPKc",
            "_ZN7android13GraphicBufferC2EjjijPKc",
            "_ZN7android13GraphicBufferC1EjjijyNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE",
            "_ZN7android13GraphicBufferC2EjjijyNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE",
            nullptr
        };
        for (const char** p = candidates_new; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!new_ctor_str) new_ctor_str = reinterpret_cast<ctor_new_str_t>(s);
            if (!new_ctor_cstr) new_ctor_cstr = reinterpret_cast<ctor_new_cstr_t>(s);
        }
        const char* candidates_stride[] = {
            "_ZN7android13GraphicBufferC1EjjijjP11native_handle",
            "_ZN7android13GraphicBufferC2EjjijjP11native_handle",
            nullptr
        };
        for (const char** p = candidates_stride; *p; ++p) {
            void* s = dlsym(h, *p);
            if (!s) continue;
            if (!ctor_stride) ctor_stride = reinterpret_cast<ctor_with_stride_t>(s);
        }
        void* d = dlsym(h, "_ZN7android13GraphicBufferD1Ev");
        if (!d) d = dlsym(h, "_ZN7android13GraphicBufferD2Ev");
        dtor = reinterpret_cast<dtor_t>(d);
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
static void call_ctor_new_str(void* _this, uint32 w, uint32 h, int32 f, uint32 u, const std::string& s)
{
    if (!g_resolvers.new_ctor_str) return;
    g_resolvers.new_ctor_str(_this, w, h, f, u, s);
}
static void call_ctor_new_cstr(void* _this, uint32 w, uint32 h, int32 f, uint32 u, const char* s)
{
    if (!g_resolvers.new_ctor_cstr) return;
    std::string tmp = s ? s : "";
    g_resolvers.new_ctor_cstr(_this, w, h, f, u, tmp.c_str());
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
    std::memset(_this, 0, sizeof(void*));
}
extern "C" void _ZN7android13GraphicBufferC1Ejjijy(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage, unsigned char arg)
{
    g_resolvers.resolve();
    if (g_resolvers.new_ctor_str) {
        call_ctor_new_str(_this, inWidth, inHeight, inFormat, inUsage, std::string(""));
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
}
