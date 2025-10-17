
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
static void* open_libgui()
{
    static std::atomic<void*> handle{nullptr};
    void* h = handle.load(std::memory_order_acquire);
    if (h) return h;
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    struct pthread_lock_guard { pthread_mutex_t* m; pthread_lock_guard(pthread_mutex_t* mm):m(mm){pthread_mutex_lock(m);} ~pthread_lock_guard(){pthread_mutex_unlock(m);} };
    pthread_lock_guard lk(&m);
    h = handle.load(std::memory_order_relaxed);
    if (h) return h;
    h = dlopen("/system/lib/libgui.so", RTLD_LAZY | RTLD_LOCAL);
    if (!h) h = dlopen("/system/lib64/libgui.so", RTLD_LAZY | RTLD_LOCAL);
    if (!h) h = dlopen("libgui.so", RTLD_LAZY | RTLD_LOCAL);
    if (!h) h = dlopen("libgui.so.0", RTLD_LAZY | RTLD_LOCAL);
    handle.store(h, std::memory_order_release);
    return h;
}
namespace shim {
using generic_fn_t = int(*)(...);
static generic_fn_t resolve_generic(const char* name)
{
    void* h = open_libgui();
    if (!h) h = open_libui();
    if (!h) return nullptr;
    void* s = dlsym(h, name);
    return (generic_fn_t)s;
}
static int try_call_generic(const char* name, uint32 a, bool b, uint32 c, uint32 d, uint32 e)
{
    generic_fn_t f = resolve_generic(name);
    if (!f) return 0;
    return f(a, (int)b, c, d, e);
}
extern "C" int _ZN7android21GuiExtClientConsumer13configDisplayEjbjjj(void* _this, uint32 a, bool b, uint32 c, uint32 d, uint32 e) asm("_ZN7android21GuiExtClientConsumer13configDisplayEjbjjj");
extern "C" int _ZN7android21GuiExtClientConsumer13configDisplayEjbjjj(void* _this, uint32 a, bool b, uint32 c, uint32 d, uint32 e)
{
    int ret = try_call_generic("_ZN7android8Composer13setDisplayStateERKNS_2spINS_21SurfaceComposerClientEEEjbjjj", a, b, c, d, e);
    if (ret) return ret;
    ret = try_call_generic("_ZN7android8Composer13setDisplayStateEjbbjjj", a, b, c, d, e);
    return 0;
}
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc(void* _this, void* s, const char* c) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc(void* _this, void* s, const char* c)
{
    generic_fn_t f = resolve_generic("_ZN7android8Composer15dumpToStringImplEv");
    if (f) f();
}
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4_(void* _this, void* a) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4_(void* _this, void* a)
{
    generic_fn_t f = resolve_generic("_ZN7android8Composer7monitorERKNS_2spINS_21SurfaceComposerClientEEE");
    if (f) f();
}
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE9unmonitorES4_(void* _this, void* a) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE9unmonitorES4");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE9unmonitorES4_(void* _this, void* a)
{
}
extern "C" void _ZN7android18BufferQueueMonitor4dumpERNS_7String8EPKc(void* _this, void* s, const char* c) asm("_ZN7android18BufferQueueMonitor4dumpERNS_7String8EPKc");
extern "C" void _ZN7android18BufferQueueMonitor4dumpERNS_7String8EPKc(void* _this, void* s, const char* c)
{
    generic_fn_t f = resolve_generic("_ZN7android21SurfaceComposerClient4dumpERNS_7String8EPKc");
    if (f) f(s, c);
}
extern "C" void* _ZN7android9SingletonINS_18BufferQueueMonitorEE11getInstanceEv() asm("_ZN7android9SingletonINS_18BufferQueueMonitorEE11getInstanceEv");
extern "C" void* _ZN7android9SingletonINS_18BufferQueueMonitorEE11getInstanceEv()
{
    generic_fn_t f = resolve_generic("_ZN7android9SingletonINS_8ComposerEE11getInstanceEv");
    if (f) return (void*)f();
    return nullptr;
}
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE7monitorES3_(void* _this, void* a) asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE7monitorES3");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE7monitorES3_(void* _this, void* a)
{
    generic_fn_t f = resolve_generic("_ZN7android7RefBase10onFirstRefEv");
    if (f) f();
}
extern "C" void _ZN7android14RefBaseMonitor4dumpERNS_7String8EPKc(void* _this, void* s, const char* c) asm("_ZN7android14RefBaseMonitor4dumpERNS_7String8EPKc");
extern "C" void _ZN7android14RefBaseMonitor4dumpERNS_7String8EPKc(void* _this, void* s, const char* c)
{
    generic_fn_t f = resolve_generic("_ZN7android7BBinder4dumpEiRKNS_6VectorINS_8String16EEE");
    if (f) f(0, s);
}
extern "C" void* _ZN7android9SingletonINS_14RefBaseMonitorEE11getInstanceEv() asm("_ZN7android9SingletonINS_14RefBaseMonitorEE11getInstanceEv");
extern "C" void* _ZN7android9SingletonINS_14RefBaseMonitorEE11getInstanceEv()
{
    generic_fn_t f = resolve_generic("_ZN7android9SingletonINS_15ComposerServiceEE11getInstanceEv");
    if (f) return (void*)f();
    return nullptr;
}
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE4dumpERNS_7String8EPKc(void* _this, void* s, const char* c) asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE4dumpERNS_7String8EPKc");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE4dumpERNS_7String8EPKc(void* _this, void* s, const char* c)
{
    generic_fn_t f = resolve_generic("_ZN7android8Composer4dumpERNS_7String8EPKc");
    if (f) f(s, c);
}
extern "C" int _ZN7android21PerfServiceController14setCPUScenarioERKb(void* _this, const bool* b) asm("_ZN7android21PerfServiceController14setCPUScenarioERKb");
extern "C" int _ZN7android21PerfServiceController14setCPUScenarioERKb(void* _this, const bool* b)
{
    return 0;
}
extern "C" void _ZN7android14FreeModeDevice11setScenarioERKj(void* _this, const unsigned int* p) asm("_ZN7android14FreeModeDevice11setScenarioERKj");
extern "C" void _ZN7android14FreeModeDevice11setScenarioERKj(void* _this, const unsigned int* p)
{
    generic_fn_t f = resolve_generic("_ZN7android8Composer11setScenarioEj");
    if (f) f(*p);
}
extern "C" void _ZN7android6Parcel9writeBoolEb(void* _this, bool b) asm("_ZN7android6Parcel9writeBoolEb");
extern "C" void _ZN7android6Parcel9writeBoolEb(void* _this, bool b)
{
    generic_fn_t f = resolve_generic("_ZN7android6Parcel9writeBoolEb");
    if (f) f(_this, (int)b);
}
extern "C" void _ZNK7android6Parcel8readBoolEPb(void* _this, unsigned char* out) asm("_ZNK7android6Parcel8readBoolEPb");
extern "C" void _ZNK7android6Parcel8readBoolEPb(void* _this, unsigned char* out)
{
    generic_fn_t f = resolve_generic("_ZNK7android6Parcel8readBoolEPb");
    if (f) f(_this, out);
}
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4(void* _this, void* a) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4(void* _this, void* a)
{
    generic_fn_t f = resolve_generic("_ZN7android8Composer7monitorERKNS_2spINS_21SurfaceComposerClientEEE");
    if (f) f();
}
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE9unmonitorES3_(void* _this, void* a) asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE9unmonitorES3_");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE9unmonitorES3_(void* _this, void* a)
{
}
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc() asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc() {}
