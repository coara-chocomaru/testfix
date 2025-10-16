#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <string>
#include <atomic>
#include <unistd.h>
#include <pthread.h>
#include <android/log.h>
#include <cstdarg>
#include <unordered_map>
using uint32 = uint32_t;
using int32 = int32_t;
static const char* TAG = "libshim_gui_combined";
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
static void* resolve_host_lib()
{
    static std::atomic<void*> handle{nullptr};
    void* h = handle.load(std::memory_order_acquire);
    if (h) return h;
    static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&g_lock);
    h = handle.load(std::memory_order_relaxed);
    if (h) { pthread_mutex_unlock(&g_lock); return h; }
    void* cand = nullptr;
    cand = open_lib("/system/lib/libgui.so", RTLD_NOW | RTLD_GLOBAL);
    if (!cand) cand = open_lib("/system/lib64/libgui.so", RTLD_NOW | RTLD_GLOBAL);
    if (!cand) cand = open_lib("/system/lib/libui.so", RTLD_NOW | RTLD_GLOBAL);
    if (!cand) cand = open_lib("/system/lib64/libui.so", RTLD_NOW | RTLD_GLOBAL);
    if (!cand) cand = open_lib("libgui.so", RTLD_NOW | RTLD_GLOBAL);
    if (!cand) cand = open_lib("libui.so", RTLD_NOW | RTLD_GLOBAL);
    handle.store(cand, std::memory_order_release);
    pthread_mutex_unlock(&g_lock);
    return cand;
}
static void* resolve_sym_cached(const char* name)
{
    static std::unordered_map<std::string, void*> cache;
    auto it = cache.find(name);
    if (it != cache.end()) return it->second;
    void* h = resolve_host_lib();
    if (!h) {
        cache[name] = nullptr;
        return nullptr;
    }
    void* s = dlsym(h, name);
    cache[name] = s;
    return s;
}
namespace shim {
using ctor_str_t = void (*)(void*, uint32, uint32, int32, uint32, std::string const&);
using ctor_cstr_t = void (*)(void*, uint32, uint32, int32, uint32, const char*);
using ctor_basic_t = void (*)(void*, uint32, uint32, int32, uint32);
using ctor_stride_t = void (*)(void*, uint32, uint32, int32, uint32, uint32, void*, bool);
using dtor_t = void (*)(void*);
struct Resolvers {
    ctor_str_t ctor_str = nullptr;
    ctor_cstr_t ctor_cstr = nullptr;
    ctor_stride_t ctor_stride = nullptr;
    ctor_basic_t ctor_basic = nullptr;
    dtor_t dtor = nullptr;
    bool resolved = false;
    void resolve()
    {
        if (resolved) return;
        resolved = true;
        void* h = resolve_host_lib();
        if (!h) {
            mylog("resolve: host lib not found");
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
            if (!ctor_basic && !strstr(*p,"basic_string") && !strstr(*p,"PKc") && !strstr(*p,"native_handle")) ctor_basic = reinterpret_cast<ctor_basic_t>(s);
            if (ctor_str && ctor_cstr && ctor_stride && ctor_basic) break;
        }
        void* d = dlsym(h, "_ZN7android13GraphicBufferD1Ev");
        if (!d) d = dlsym(h, "_ZN7android13GraphicBufferD2Ev");
        dtor = reinterpret_cast<dtor_t>(d);
        mylog("resolve: ctor_str=%p ctor_cstr=%p ctor_stride=%p ctor_basic=%p dtor=%p", ctor_str, ctor_cstr, ctor_stride, ctor_basic, dtor);
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
static void call_ctor_basic(void* _this, uint32 w, uint32 h, int32 f, uint32 u)
{
    if (!g_resolvers.ctor_basic) return;
    g_resolvers.ctor_basic(_this, w, h, f, u);
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
    if (g_resolvers.ctor_basic) {
        call_ctor_basic(_this, inWidth, inHeight, inFormat, inUsage);
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
    if (g_resolvers.ctor_basic) {
        call_ctor_basic(_this, inWidth, inHeight, inFormat, inUsage);
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
    if (g_resolvers.ctor_basic) {
        call_ctor_basic(_this, inWidth, inHeight, inFormat, inUsage);
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
    if (g_resolvers.ctor_basic) {
        call_ctor_basic(_this, inWidth, inHeight, inFormat, inUsage);
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
    if (g_resolvers.ctor_basic) {
        call_ctor_basic(_this, inWidth, inHeight, inFormat, inUsage);
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
    if (g_resolvers.ctor_basic) {
        call_ctor_basic(_this, inWidth, inHeight, inFormat, inUsage);
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
extern "C" void _ZN7android11BufferQueue17createBufferQueueEPNS_2spINS_22IGraphicBufferProducerEEEPNS1_INS_22IGraphicBufferConsumerEEERKNS1_INS_19IGraphicBufferAllocEEE(void* outProducer, void* outConsumer, const void* allocator) asm("_ZN7android11BufferQueue17createBufferQueueEPNS_2spINS_22IGraphicBufferProducerEEEPNS1_INS_22IGraphicBufferConsumerEEERKNS1_INS_19IGraphicBufferAllocEEE");
void _ZN7android11BufferQueue17createBufferQueueEPNS_2spINS_22IGraphicBufferProducerEEEPNS1_INS_22IGraphicBufferConsumerEEERKNS1_INS_19IGraphicBufferAllocEEE(void* outProducer, void* outConsumer, const void* allocator)
{
    const char* sym = "_ZN7android11BufferQueue17createBufferQueueEPNS_2spINS_22IGraphicBufferProducerEEEPNS1_INS_22IGraphicBufferConsumerEEERKNS1_INS_19IGraphicBufferAllocEEE";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*, void*, const void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(outProducer, outConsumer, allocator);
        return;
    }
    mylog("fallback createBufferQueue called");
    if (outProducer) *((void**)outProducer) = nullptr;
    if (outConsumer) *((void**)outConsumer) = nullptr;
    return;
}
extern "C" void _ZN7android11BufferQueue21ProxyConsumerListener16onFrameAvailableERKNS_10BufferItemE(void* _this, const void* item) asm("_ZN7android11BufferQueue21ProxyConsumerListener16onFrameAvailableERKNS_10BufferItemE");
void _ZN7android11BufferQueue21ProxyConsumerListener16onFrameAvailableERKNS_10BufferItemE(void* _this, const void* item)
{
    const char* sym = "_ZN7android11BufferQueue21ProxyConsumerListener16onFrameAvailableERKNS_10BufferItemE";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*, const void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this, item);
        return;
    }
    mylog("fallback ProxyConsumerListener::onFrameAvailable for %p", _this);
    return;
}
extern "C" void _ZN7android11BufferQueue21ProxyConsumerListener15onFrameReplacedERKNS_10BufferItemE(void* _this, const void* item) asm("_ZN7android11BufferQueue21ProxyConsumerListener15onFrameReplacedERKNS_10BufferItemE");
void _ZN7android11BufferQueue21ProxyConsumerListener15onFrameReplacedERKNS_10BufferItemE(void* _this, const void* item)
{
    const char* sym = "_ZN7android11BufferQueue21ProxyConsumerListener15onFrameReplacedERKNS_10BufferItemE";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*, const void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this, item);
        return;
    }
    mylog("fallback ProxyConsumerListener::onFrameReplaced for %p", _this);
    return;
}
extern "C" void _ZN7android11BufferQueue21ProxyConsumerListener17onBuffersReleasedEv(void* _this) asm("_ZN7android11BufferQueue21ProxyConsumerListener17onBuffersReleasedEv");
void _ZN7android11BufferQueue21ProxyConsumerListener17onBuffersReleasedEv(void* _this)
{
    const char* sym = "_ZN7android11BufferQueue21ProxyConsumerListener17onBuffersReleasedEv";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this);
        return;
    }
    mylog("fallback ProxyConsumerListener::onBuffersReleased for %p", _this);
    return;
}
extern "C" void _ZN7android11BufferQueue21ProxyConsumerListener23onSidebandStreamChangedEv(void* _this) asm("_ZN7android11BufferQueue21ProxyConsumerListener23onSidebandStreamChangedEv");
void _ZN7android11BufferQueue21ProxyConsumerListener23onSidebandStreamChangedEv(void* _this)
{
    const char* sym = "_ZN7android11BufferQueue21ProxyConsumerListener23onSidebandStreamChangedEv";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this);
        return;
    }
    mylog("fallback ProxyConsumerListener::onSidebandStreamChanged for %p", _this);
    return;
}
extern "C" void _ZN7android11BufferQueue21ProxyConsumerListenerC1ERKNS_2wpINS_16ConsumerListenerEEE(void* _this, const void* wp) asm("_ZN7android11BufferQueue21ProxyConsumerListenerC1ERKNS_2wpINS_16ConsumerListenerEEE");
void _ZN7android11BufferQueue21ProxyConsumerListenerC1ERKNS_2wpINS_16ConsumerListenerEEE(void* _this, const void* wp)
{
    const char* sym = "_ZN7android11BufferQueue21ProxyConsumerListenerC1ERKNS_2wpINS_16ConsumerListenerEEE";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*, const void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this, wp);
        return;
    }
    mylog("fallback ProxyConsumerListener C1 for %p", _this);
    memset(_this, 0, sizeof(void*));
    return;
}
extern "C" void _ZN7android11BufferQueue21ProxyConsumerListenerC2ERKNS_2wpINS_16ConsumerListenerEEE(void* _this, const void* wp) asm("_ZN7android11BufferQueue21ProxyConsumerListenerC2ERKNS_2wpINS_16ConsumerListenerEEE");
void _ZN7android11BufferQueue21ProxyConsumerListenerC2ERKNS_2wpINS_16ConsumerListenerEEE(void* _this, const void* wp)
{
    const char* sym = "_ZN7android11BufferQueue21ProxyConsumerListenerC2ERKNS_2wpINS_16ConsumerListenerEEE";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*, const void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this, wp);
        return;
    }
    mylog("fallback ProxyConsumerListener C2 for %p", _this);
    memset(_this, 0, sizeof(void*));
    return;
}
extern "C" void _ZN7android11BufferQueue21ProxyConsumerListenerD0Ev(void* _this) asm("_ZN7android11BufferQueue21ProxyConsumerListenerD0Ev");
void _ZN7android11BufferQueue21ProxyConsumerListenerD0Ev(void* _this)
{
    const char* sym = "_ZN7android11BufferQueue21ProxyConsumerListenerD0Ev";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this);
        return;
    }
    mylog("fallback ProxyConsumerListener D0 for %p", _this);
    return;
}
extern "C" void _ZN7android11BufferQueue21ProxyConsumerListenerD1Ev(void* _this) asm("_ZN7android11BufferQueue21ProxyConsumerListenerD1Ev");
void _ZN7android11BufferQueue21ProxyConsumerListenerD1Ev(void* _this)
{
    const char* sym = "_ZN7android11BufferQueue21ProxyConsumerListenerD1Ev";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this);
        return;
    }
    mylog("fallback ProxyConsumerListener D1 for %p", _this);
    return;
}
extern "C" void _ZN7android11BufferQueue21ProxyConsumerListenerD2Ev(void* _this) asm("_ZN7android11BufferQueue21ProxyConsumerListenerD2Ev");
void _ZN7android11BufferQueue21ProxyConsumerListenerD2Ev(void* _this)
{
    const char* sym = "_ZN7android11BufferQueue21ProxyConsumerListenerD2Ev";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this);
        return;
    }
    mylog("fallback ProxyConsumerListener D2 for %p", _this);
    return;
}
extern "C" void _ZN7android19BufferQueueConsumerC2ERKNS_2spINS_15BufferQueueCoreEEE(void* _this, const void* sp) asm("_ZN7android19BufferQueueConsumerC2ERKNS_2spINS_15BufferQueueCoreEEE");
void _ZN7android19BufferQueueConsumerC2ERKNS_2spINS_15BufferQueueCoreEEE(void* _this, const void* sp)
{
    const char* sym = "_ZN7android19BufferQueueConsumerC2ERKNS_2spINS_15BufferQueueCoreEEE";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*, const void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this, sp);
        return;
    }
    mylog("fallback BufferQueueConsumer C2 for %p", _this);
    memset(_this, 0, sizeof(void*));
    return;
}
extern "C" void _ZN7android19BufferQueueConsumerD2Ev(void* _this) asm("_ZN7android19BufferQueueConsumerD2Ev");
void _ZN7android19BufferQueueConsumerD2Ev(void* _this)
{
    const char* sym = "_ZN7android19BufferQueueConsumerD2Ev";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this);
        return;
    }
    mylog("fallback BufferQueueConsumer D2 for %p", _this);
    return;
}
extern "C" void _ZN7android19BufferQueueConsumerD1Ev(void* _this) asm("_ZN7android19BufferQueueConsumerD1Ev");
void _ZN7android19BufferQueueConsumerD1Ev(void* _this)
{
    const char* sym = "_ZN7android19BufferQueueConsumerD1Ev";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this);
        return;
    }
    mylog("fallback BufferQueueConsumer D1 for %p", _this);
    return;
}
extern "C" void _ZN7android19BufferQueueProducerC1ERKNS_2spINS_15BufferQueueCoreEEE(void* _this, const void* sp) asm("_ZN7android19BufferQueueProducerC1ERKNS_2spINS_15BufferQueueCoreEEE");
void _ZN7android19BufferQueueProducerC1ERKNS_2spINS_15BufferQueueCoreEEE(void* _this, const void* sp)
{
    const char* sym = "_ZN7android19BufferQueueProducerC1ERKNS_2spINS_15BufferQueueCoreEEE";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*, const void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this, sp);
        return;
    }
    mylog("fallback BufferQueueProducer C1 for %p", _this);
    memset(_this, 0, sizeof(void*));
    return;
}
extern "C" void _ZN7android19BufferQueueProducerD2Ev(void* _this) asm("_ZN7android19BufferQueueProducerD2Ev");
void _ZN7android19BufferQueueProducerD2Ev(void* _this)
{
    const char* sym = "_ZN7android19BufferQueueProducerD2Ev";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this);
        return;
    }
    mylog("fallback BufferQueueProducer D2 for %p", _this);
    return;
}
extern "C" void _ZN7android19BufferQueueProducerD1Ev(void* _this) asm("_ZN7android19BufferQueueProducerD1Ev");
void _ZN7android19BufferQueueProducerD1Ev(void* _this)
{
    const char* sym = "_ZN7android19BufferQueueProducerD1Ev";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this);
        return;
    }
    mylog("fallback BufferQueueProducer D1 for %p", _this);
    return;
}
extern "C" void _ZN7android15BufferQueueCoreC1ERKNS_2spINS_19IGraphicBufferAllocEEE(void* _this, const void* sp) asm("_ZN7android15BufferQueueCoreC1ERKNS_2spINS_19IGraphicBufferAllocEEE");
void _ZN7android15BufferQueueCoreC1ERKNS_2spINS_19IGraphicBufferAllocEEE(void* _this, const void* sp)
{
    const char* sym = "_ZN7android15BufferQueueCoreC1ERKNS_2spINS_19IGraphicBufferAllocEEE";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*, const void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this, sp);
        return;
    }
    mylog("fallback BufferQueueCore C1 for %p", _this);
    memset(_this, 0, sizeof(void*));
    return;
}
extern "C" void _ZN7android15BufferQueueCoreC2ERKNS_2spINS_19IGraphicBufferAllocEEE(void* _this, const void* sp) asm("_ZN7android15BufferQueueCoreC2ERKNS_2spINS_19IGraphicBufferAllocEEE");
void _ZN7android15BufferQueueCoreC2ERKNS_2spINS_19IGraphicBufferAllocEEE(void* _this, const void* sp)
{
    const char* sym = "_ZN7android15BufferQueueCoreC2ERKNS_2spINS_19IGraphicBufferAllocEEE";
    void* fptr = resolve_sym_cached(sym);
    if (fptr) {
        typedef void (*fn_t)(void*, const void*);
        fn_t fn = reinterpret_cast<fn_t>(fptr);
        fn(_this, sp);
        return;
    }
    mylog("fallback BufferQueueCore C2 for %p", _this);
    memset(_this, 0, sizeof(void*));
    return;
}
} // extern "C"
__attribute__((constructor)) static void shim_init(void)
{
    void* h = resolve_host_lib();
    if (h) {
        mylog("shim_init: host lib loaded %p", h);
    } else {
        mylog("shim_init: failed to load host lib");
    }
    void* eg = dlopen("libEGL.so", RTLD_NOW | RTLD_GLOBAL);
    void* gles = dlopen("libGLESv2.so", RTLD_NOW | RTLD_GLOBAL);
    mylog("shim_init: libEGL=%p libGLESv2=%p", eg, gles);
}
