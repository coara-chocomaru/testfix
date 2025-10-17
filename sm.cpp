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
static void* open_libui(){
    static std::atomic<void*> handle{nullptr};
    void* h = handle.load(std::memory_order_acquire);
    if(h) return h;
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    struct pthread_lock_guard{ pthread_mutex_t* m; pthread_lock_guard(pthread_mutex_t* mm):m(mm){pthread_mutex_lock(m);} ~pthread_lock_guard(){pthread_mutex_unlock(m);} };
    pthread_lock_guard lk(&m);
    h = handle.load(std::memory_order_relaxed);
    if(h) return h;
    h = dlopen("/system/lib/libui.so", RTLD_LAZY | RTLD_LOCAL);
    if(!h) h = dlopen("/system/lib64/libui.so", RTLD_LAZY | RTLD_LOCAL);
    if(!h) h = dlopen("libui.so", RTLD_LAZY | RTLD_LOCAL);
    if(!h) h = dlopen("libui.so.0", RTLD_LAZY | RTLD_LOCAL);
    handle.store(h, std::memory_order_release);
    return h;
}
static void* open_libgui(){
    static std::atomic<void*> handle{nullptr};
    void* h = handle.load(std::memory_order_acquire);
    if(h) return h;
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    struct pthread_lock_guard{ pthread_mutex_t* m; pthread_lock_guard(pthread_mutex_t* mm):m(mm){pthread_mutex_lock(m);} ~pthread_lock_guard(){pthread_mutex_unlock(m);} };
    pthread_lock_guard lk(&m);
    h = handle.load(std::memory_order_relaxed);
    if(h) return h;
    h = dlopen("/system/lib/libgui.so", RTLD_LAZY | RTLD_LOCAL);
    if(!h) h = dlopen("/system/lib64/libgui.so", RTLD_LAZY | RTLD_LOCAL);
    if(!h) h = dlopen("libgui.so", RTLD_LAZY | RTLD_LOCAL);
    if(!h) h = dlopen("libgui.so.0", RTLD_LAZY | RTLD_LOCAL);
    handle.store(h, std::memory_order_release);
    return h;
}
namespace shim{
using cfg_consumer_t = void (*)(void*, uint32, bool, uint32, uint32, uint32);
static cfg_consumer_t real_configDisplay = nullptr;
static void resolve_configDisplay(){
    if(real_configDisplay) return;
    void* hg = open_libgui();
    if(!hg) hg = open_libui();
    if(!hg) return;
    real_configDisplay = (cfg_consumer_t)dlsym(hg, "_ZN7android21GuiExtClientConsumer13configDisplayEjbjjj");
}
extern "C" void _ZN7android21GuiExtClientConsumer13configDisplayEjbjjj(void* _this, uint32 a, bool b, uint32 c, uint32 d, uint32 e) asm("_ZN7android21GuiExtClientConsumer13configDisplayEjbjjj");
extern "C" void _ZN7android21GuiExtClientConsumer13configDisplayEjbjjj(void* _this, uint32 a, bool b, uint32 c, uint32 d, uint32 e){
    resolve_configDisplay();
    if(real_configDisplay){ real_configDisplay(_this, a, b, c, d, e); return; }
}
using fnptr_t = void(*)();
static fnptr_t try_resolve(const char* name){
    void* h = open_libgui();
    if(!h) h = open_libui();
    if(!h) return nullptr;
    void* s = dlsym(h, name);
    return (fnptr_t)s;
}
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE14getProcessNameEv() asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE14getProcessNameEv");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE14getProcessNameEv(){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE14getProcessNameEv"); if(f) f(); }
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE4dumpERNS_7String8EPKc(void* _this, void* s, const char* c) asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE4dumpERNS_7String8EPKc");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE4dumpERNS_7String8EPKc(void* _this, void* s, const char* c){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE4dumpERNS_7String8EPKc"); if(f) ((void(*)(void*,void*,const char*))f)(_this,s,c); }
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE7monitorES3_(void* _this, void* a) asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE7monitorES3_");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE7monitorES3_(void* _this, void* a){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE7monitorES3_"); if(f) ((void(*)(void*,void*))f)(_this,a); }
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE9unmonitorES3_(void* _this, void* a) asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE9unmonitorES3_");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE9unmonitorES3_(void* _this, void* a){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE9unmonitorES3_"); if(f) ((void(*)(void*,void*))f)(_this,a); }
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED0Ev(void* _this) asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED0Ev");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED0Ev(void* _this){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED0Ev"); if(f) ((void(*)(void*))f)(_this); }
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED2Ev(void* _this) asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED2Ev");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED2Ev(void* _this){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED2Ev"); if(f) ((void(*)(void*))f)(_this); }
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE14getProcessNameEv() asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE14getProcessNameEv");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE14getProcessNameEv(){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE14getProcessNameEv"); if(f) f(); }
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc(void* _this, void* s, const char* c) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc(void* _this, void* s, const char* c){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc"); if(f) ((void(*)(void*,void*,const char*))f)(_this,s,c); }
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4_(void* _this, void* a) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4_");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4_(void* _this, void* a){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4_"); if(f) ((void(*)(void*,void*))f)(_this,a); }
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE9unmonitorES4_(void* _this, void* a) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE9unmonitorES4_");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE9unmonitorES4_(void* _this, void* a){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE9unmonitorES4_"); if(f) ((void(*)(void*,void*))f)(_this,a); }
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED0Ev(void* _this) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED0Ev");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED0Ev(void* _this){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED0Ev"); if(f) ((void(*)(void*))f)(_this); }
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED2Ev(void* _this) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED2Ev");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED2Ev(void* _this){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED2Ev"); if(f) ((void(*)(void*))f)(_this); }
extern "C" void _ZNK7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE10getKeyNameEv() asm("_ZNK7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE10getKeyNameEv");
extern "C" void _ZNK7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE10getKeyNameEv(){ fnptr_t f = try_resolve("_ZNK7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE10getKeyNameEv"); if(f) f(); }
extern "C" void _ZNK7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE10getKeyNameEv() asm("_ZNK7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE10getKeyNameEv");
extern "C" void _ZNK7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE10getKeyNameEv(){ fnptr_t f = try_resolve("_ZNK7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE10getKeyNameEv"); if(f) f(); }
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
    void resolve(){ if(resolved) return; resolved = true; void* h = open_libui(); void* hg = open_libgui(); if(!h && !hg) return;
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC2EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC1EjjijPKc");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC2EjjijPKc");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC1EjjijjP13native_handle");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC2EjjijjP13native_handle");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC1EP19ANativeWindowBufferb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC2EP19ANativeWindowBufferb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferD1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferD2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer8initSizeEjjij");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer8initSizeEjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer10reallocateEjjij");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer11free_handleEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer4lockEjPPv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer4lockEjRKNS_4RectEPPv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer6unlockEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer15getNativeBufferEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer10getFdCountEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android10GLConsumer19releaseBufferLockedEiNS_2spINS_13GraphicBufferEEEPvS4_");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android10GLConsumer22computeTransformMatrixEPfRKNS_2spINS_13GraphicBufferEEERKNS_4RectEjb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android10GLConsumer8EglImage11createImageEPvRKNS_2spINS_13GraphicBufferEEERKNS_4RectE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android10GLConsumer8EglImageC1ENS_2spINS_13GraphicBufferEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android10GLConsumer8EglImageC2ENS_2spINS_13GraphicBufferEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android10GLConsumerC1ERKNS_2spINS_22IGraphicBufferConsumerEEEjbb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android10GLConsumerC1ERKNS_2spINS_22IGraphicBufferConsumerEEEjjbb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android10GLConsumerC2ERKNS_2spINS_22IGraphicBufferConsumerEEEjbb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android10GLConsumerC2ERKNS_2spINS_22IGraphicBufferConsumerEEEjjbb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android11BnInterfaceINS_19IGraphicBufferAllocEE10onAsBinderEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android11BnInterfaceINS_22IGraphicBufferProducerEE10onAsBinderEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android11BufferQueue17createBufferQueueEPNS_2spINS_22IGraphicBufferProducerEEEPNS1_INS_22IGraphicBufferConsumerEEERKNS1_INS_19IGraphicBufferAllocEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android11CpuConsumerC1ERKNS_2spINS_22IGraphicBufferConsumerEEEmb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android11CpuConsumerC2ERKNS_2spINS_22IGraphicBufferConsumerEEEmb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android12ConsumerBase13stillTrackingEiNS_2spINS_13GraphicBufferEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android12ConsumerBase15addReleaseFenceEiNS_2spINS_13GraphicBufferEEERKNS1_INS_5FenceEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android12ConsumerBase19releaseBufferLockedEiNS_2spINS_13GraphicBufferEEEPvS4_");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android12ConsumerBase21addReleaseFenceLockedEiNS_2spINS_13GraphicBufferEEERKNS1_INS_5FenceEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android12ConsumerBaseC1ERKNS_2spINS_22IGraphicBufferConsumerEEEb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android12ConsumerBaseC2ERKNS_2spINS_22IGraphicBufferConsumerEEEb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer10reallocateEjjij");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer11free_handleEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer11unlockAsyncEPi");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer14lockAsyncYCbCrEjP13android_ycbcri");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer14lockAsyncYCbCrEjRKNS_4RectEP13android_ycbcri");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer17needsReallocationEjjij");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer26dumpAllocationsToSystemLogEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer4lockEjPPv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer4lockEjRKNS_4RectEPPv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer6unlockEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer8initSizeEjjij");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer9lockAsyncEjPPvi");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer9lockAsyncEjRKNS_4RectEPPvi");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer9lockYCbCrEjP13android_ycbcr");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer9lockYCbCrEjRKNS_4RectEP13android_ycbcr");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBuffer9unflattenERPKvRmRPKiS4_");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC1EP19ANativeWindowBufferb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC1Ejjij");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC1EjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC1EjjijjP13native_handleb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC2EP19ANativeWindowBufferb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC2Ejjij");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC2EjjijjP13native_handleb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferC2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferD0Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferD1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android13GraphicBufferD2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android14StreamSplitter13BufferTrackerC1ERKNS_2spINS_13GraphicBufferEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android14StreamSplitter13BufferTrackerC2ERKNS_2spINS_13GraphicBufferEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android14StreamSplitter14OutputListenerC1ERKNS_2spIS0_EERKNS2_INS_22IGraphicBufferProducerEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android14StreamSplitter14OutputListenerC2ERKNS_2spIS0_EERKNS2_INS_22IGraphicBufferProducerEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android14StreamSplitter14createSplitterERKNS_2spINS_22IGraphicBufferConsumerEEEPNS1_IS0_EE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android14StreamSplitter24onBufferReleasedByOutputERKNS_2spINS_22IGraphicBufferProducerEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android14StreamSplitter9addOutputERKNS_2spINS_22IGraphicBufferProducerEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android14StreamSplitterC1ERKNS_2spINS_22IGraphicBufferConsumerEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android14StreamSplitterC2ERKNS_2spINS_22IGraphicBufferConsumerEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android14SurfaceControlC1ERKNS_2spINS_21SurfaceComposerClientEEERKNS1_INS_7IBinderEEERKNS1_INS_22IGraphicBufferProducerEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android14SurfaceControlC2ERKNS_2spINS_21SurfaceComposerClientEEERKNS1_INS_7IBinderEEERKNS1_INS_22IGraphicBufferProducerEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android15BufferQueueCoreC1ERKNS_2spINS_19IGraphicBufferAllocEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android15BufferQueueCoreC2ERKNS_2spINS_19IGraphicBufferAllocEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android15BufferQueueDump15onAcquireBufferERKiRKNS_2spINS_13GraphicBufferEEERKNS3_INS_5FenceEEERKlRKj");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android15BufferQueueDump15onDequeueBufferERKiRKNS_2spINS_13GraphicBufferEEERKNS3_INS_5FenceEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android15BufferQueueDump9addBufferERKiRKNS_2spINS_13GraphicBufferEEERKNS3_INS_5FenceEEERKlRKj");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android16BufferQueueDebug10setIonInfoERKNS_2spINS_13GraphicBufferEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android16BufferQueueDebug7onQueueEiRKNS_2spINS_13GraphicBufferEEElRKNS1_INS_5FenceEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android16BufferQueueDebug9onAcquireEiRKNS_2spINS_13GraphicBufferEEERKNS1_INS_5FenceEEERKlRKjPKNS_10BufferItemE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android16BufferQueueDebug9onDequeueEiRNS_2spINS_13GraphicBufferEEERNS1_INS_5FenceEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android16ScreenshotClient7captureERKNS_2spINS_7IBinderEEERKNS1_INS_22IGraphicBufferProducerEEENS_4RectEjjjjb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android17GraphicBufferUtil14downSampleCopyERKNS_16DownSampleConfigERKNS_2spINS_13GraphicBufferEEERS6_");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android17GraphicBufferUtil15getBitsPerPixelEi");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android17GraphicBufferUtil4dumpERKNS_2spINS_13GraphicBufferEEEPKcS7_");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android17GraphicBufferUtil8drawLineERKNS_2spINS_13GraphicBufferEEEhiii");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android17GraphicBufferUtilC1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android18BufferItemConsumerC1ERKNS_2spINS_22IGraphicBufferConsumerEEEjib");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android18BufferItemConsumerC2ERKNS_2spINS_22IGraphicBufferConsumerEEEjib");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android18GraphicBufferAlloc19createGraphicBufferEjjijNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEEPi");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android18GraphicBufferAlloc19createGraphicBufferEjjijPi");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android18GraphicBufferAllocC1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android18GraphicBufferAllocC2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android18GraphicBufferAllocD0Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android18GraphicBufferAllocD1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android18GraphicBufferAllocD2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19BufferQueueConsumer12attachBufferEPiRKNS_2spINS_13GraphicBufferEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19BufferQueueProducer10disconnectEiNS_22IGraphicBufferProducer14DisconnectModeE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19BufferQueueProducer11queueBufferEiRKNS_22IGraphicBufferProducer16QueueBufferInputEPNS1_17QueueBufferOutputE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19BufferQueueProducer12attachBufferEPiRKNS_2spINS_13GraphicBufferEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19BufferQueueProducer13requestBufferEiPNS_2spINS_13GraphicBufferEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19BufferQueueProducer16detachNextBufferEPNS_2spINS_13GraphicBufferEEEPNS1_INS_5FenceEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19BufferQueueProducer19getLastQueuedBufferEPNS_2spINS_13GraphicBufferEEEPNS1_INS_5FenceEEEPf");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19BufferQueueProducer7connectERKNS_2spINS_17IProducerListenerEEEibPNS_22IGraphicBufferProducer17QueueBufferOutputE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19GraphicBufferMapper11unlockAsyncEPK13native_handlePi");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19GraphicBufferMapper14lockAsyncYCbCrEPK13native_handlejRKNS_4RectEP13android_ycbcri");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19GraphicBufferMapper14registerBufferEPK13native_handle");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19GraphicBufferMapper16unregisterBufferEPK13native_handle");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19GraphicBufferMapper4lockEPK13native_handlejRKNS_4RectEPPv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19GraphicBufferMapper6unlockEPK13native_handle");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19GraphicBufferMapper9lockAsyncEPK13native_handlejRKNS_4RectEPPvi");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19GraphicBufferMapper9lockYCbCrEPK13native_handlejRKNS_4RectEP13android_ycbcr");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19GraphicBufferMapperC1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19GraphicBufferMapperC2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19IGraphicBufferAlloc10descriptorE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19IGraphicBufferAlloc11asInterfaceERKNS_2spINS_7IBinderEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19IGraphicBufferAllocC2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19IGraphicBufferAllocD0Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19IGraphicBufferAllocD1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android19IGraphicBufferAllocD2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android20BnGraphicBufferAlloc10onTransactEjRKNS_6ParcelEPS1_j");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android20BpGraphicBufferAllocD0Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android20BpGraphicBufferAllocD1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android20BpGraphicBufferAllocD2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android21SurfaceComposerClient17setDisplaySurfaceERKNS_2spINS_7IBinderEEENS1_INS_22IGraphicBufferProducerEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android21SurfaceComposerClient17setDisplaySurfaceERKNS_2spINS_7IBinderEEERKNS1_INS_22IGraphicBufferProducerEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22GraphicBufferAllocator10sAllocListE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22GraphicBufferAllocator15dumpToSystemLogEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22GraphicBufferAllocator4freeEPK13native_handle");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22GraphicBufferAllocator5allocEjjijPPK13native_handlePj");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22GraphicBufferAllocator5sLockE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22GraphicBufferAllocatorC1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22GraphicBufferAllocatorC2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22GraphicBufferAllocatorD1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22GraphicBufferAllocatorD2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22IGraphicBufferConsumerC2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22IGraphicBufferConsumerD0Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22IGraphicBufferConsumerD1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22IGraphicBufferConsumerD2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22IGraphicBufferProducer10descriptorE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22IGraphicBufferProducer11asInterfaceERKNS_2spINS_7IBinderEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22IGraphicBufferProducer16QueueBufferInput9unflattenERPKvRmRPKiS5_");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22IGraphicBufferProducerC2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22IGraphicBufferProducerD0Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22IGraphicBufferProducerD1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android22IGraphicBufferProducerD2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android23BnGraphicBufferConsumer10onTransactEjRKNS_6ParcelEPS1_j");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android23BnGraphicBufferProducer10onTransactEjRKNS_6ParcelEPS1_j");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android23BpGraphicBufferConsumerD0Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android23BpGraphicBufferConsumerD1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android23BpGraphicBufferConsumerD2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android23BpGraphicBufferProducerD0Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android23BpGraphicBufferProducerD1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android23BpGraphicBufferProducerD2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android2spINS_13GraphicBufferEED2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android7Surface10disconnectEiNS_22IGraphicBufferProducer14DisconnectModeE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android7Surface16detachNextBufferEPNS_2spINS_13GraphicBufferEEEPNS1_INS_5FenceEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android7Surface19getLastQueuedBufferEPNS_2spINS_13GraphicBufferEEEPNS1_INS_5FenceEEEPf");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android7SurfaceC1ERKNS_2spINS_22IGraphicBufferProducerEEEb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android7SurfaceC2ERKNS_2spINS_22IGraphicBufferProducerEEEb");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android8Composer17setDisplaySurfaceERKNS_2spINS_7IBinderEEENS1_INS_22IGraphicBufferProducerEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android8Composer17setDisplaySurfaceERKNS_2spINS_7IBinderEEERKNS1_INS_22IGraphicBufferProducerEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_17GraphicBufferUtilEE5sLockE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_17GraphicBufferUtilEE9sInstanceE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_19GraphicBufferMapperEE11getInstanceEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_19GraphicBufferMapperEE11hasInstanceEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_19GraphicBufferMapperEE5sLockE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_19GraphicBufferMapperEE9sInstanceE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_19GraphicBufferMapperEEC1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_19GraphicBufferMapperEEC2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_19GraphicBufferMapperEED1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_19GraphicBufferMapperEED2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_22GraphicBufferAllocatorEE11getInstanceEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_22GraphicBufferAllocatorEE11hasInstanceEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_22GraphicBufferAllocatorEE5sLockE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_22GraphicBufferAllocatorEE9sInstanceE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_22GraphicBufferAllocatorEEC1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_22GraphicBufferAllocatorEEC2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_22GraphicBufferAllocatorEED1Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZN7android9SingletonINS_22GraphicBufferAllocatorEED2Ev");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android12SortedVectorINS_16key_value_pair_tIPK13native_handleNS_22GraphicBufferAllocator11alloc_rec_tEEEE10do_compareEPKvSA_");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android12SortedVectorINS_16key_value_pair_tIPK13native_handleNS_22GraphicBufferAllocator11alloc_rec_tEEEE10do_destroyEPvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android12SortedVectorINS_16key_value_pair_tIPK13native_handleNS_22GraphicBufferAllocator11alloc_rec_tEEEE12do_constructEPvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android12SortedVectorINS_16key_value_pair_tIPK13native_handleNS_22GraphicBufferAllocator11alloc_rec_tEEEE15do_move_forwardEPvPKvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android12SortedVectorINS_16key_value_pair_tIPK13native_handleNS_22GraphicBufferAllocator11alloc_rec_tEEEE16do_move_backwardEPvPKvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android12SortedVectorINS_16key_value_pair_tIPK13native_handleNS_22GraphicBufferAllocator11alloc_rec_tEEEE7do_copyEPvPKvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android12SortedVectorINS_16key_value_pair_tIPK13native_handleNS_22GraphicBufferAllocator11alloc_rec_tEEEE8do_splatEPvPKvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android13GraphicBuffer10getFdCountEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android13GraphicBuffer15getNativeBufferEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android13GraphicBuffer16getFlattenedSizeEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android13GraphicBuffer7flattenERPvRmRPiS3_");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android13GraphicBuffer9initCheckEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android19IGraphicBufferAlloc22getInterfaceDescriptorEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android22GraphicBufferAllocator4dumpERNS_7String8E");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android22IGraphicBufferConsumer22getInterfaceDescriptorEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android22IGraphicBufferProducer16QueueBufferInput10getFdCountEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android22IGraphicBufferProducer16QueueBufferInput16getFlattenedSizeEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android22IGraphicBufferProducer16QueueBufferInput7flattenERPvRmRPiS4_");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android22IGraphicBufferProducer22getInterfaceDescriptorEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android6VectorINS_2spINS_13GraphicBufferEEEE10do_destroyEPvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android6VectorINS_2spINS_13GraphicBufferEEEE12do_constructEPvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android6VectorINS_2spINS_13GraphicBufferEEEE15do_move_forwardEPvPKvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android6VectorINS_2spINS_13GraphicBufferEEEE16do_move_backwardEPvPKvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android6VectorINS_2spINS_13GraphicBufferEEEE7do_copyEPvPKvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android6VectorINS_2spINS_13GraphicBufferEEEE8do_splatEPvPKvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android6VectorINS_2spINS_22IGraphicBufferProducerEEEE10do_destroyEPvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android6VectorINS_2spINS_22IGraphicBufferProducerEEEE12do_constructEPvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android6VectorINS_2spINS_22IGraphicBufferProducerEEEE15do_move_forwardEPvPKvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android6VectorINS_2spINS_22IGraphicBufferProducerEEEE16do_move_backwardEPvPKvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android6VectorINS_2spINS_22IGraphicBufferProducerEEEE7do_copyEPvPKvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android6VectorINS_2spINS_22IGraphicBufferProducerEEEE8do_splatEPvPKvm");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZNK7android7Surface25getIGraphicBufferProducerEv");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BnInterfaceINS_19IGraphicBufferAllocEEE0_NS_10IInterfaceE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BnInterfaceINS_19IGraphicBufferAllocEEE0_NS_19IGraphicBufferAllocE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BnInterfaceINS_19IGraphicBufferAllocEEE8_NS_7BBinderE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BnInterfaceINS_19IGraphicBufferAllocEEE8_NS_7IBinderE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BnInterfaceINS_22IGraphicBufferConsumerEEE0_NS_10IInterfaceE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BnInterfaceINS_22IGraphicBufferConsumerEEE0_NS_22IGraphicBufferConsumerE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BnInterfaceINS_22IGraphicBufferConsumerEEE8_NS_7BBinderE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BnInterfaceINS_22IGraphicBufferConsumerEEE8_NS_7IBinderE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BnInterfaceINS_22IGraphicBufferProducerEEE0_NS_10IInterfaceE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BnInterfaceINS_22IGraphicBufferProducerEEE0_NS_22IGraphicBufferProducerE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BnInterfaceINS_22IGraphicBufferProducerEEE8_NS_7BBinderE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BnInterfaceINS_22IGraphicBufferProducerEEE8_NS_7IBinderE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BpInterfaceINS_19IGraphicBufferAllocEEE0_NS_10IInterfaceE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BpInterfaceINS_19IGraphicBufferAllocEEE0_NS_19IGraphicBufferAllocE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BpInterfaceINS_19IGraphicBufferAllocEEE8_NS_9BpRefBaseE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BpInterfaceINS_22IGraphicBufferConsumerEEE0_NS_10IInterfaceE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BpInterfaceINS_22IGraphicBufferConsumerEEE0_NS_22IGraphicBufferConsumerE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BpInterfaceINS_22IGraphicBufferConsumerEEE8_NS_9BpRefBaseE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BpInterfaceINS_22IGraphicBufferProducerEEE0_NS_10IInterfaceE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BpInterfaceINS_22IGraphicBufferProducerEEE0_NS_22IGraphicBufferProducerE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android11BpInterfaceINS_22IGraphicBufferProducerEEE8_NS_9BpRefBaseE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android18GraphicBufferAllocE0_NS_10IInterfaceE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android18GraphicBufferAllocE0_NS_11BnInterfaceINS_19IGraphicBufferAllocEEE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android18GraphicBufferAllocE0_NS_19IGraphicBufferAllocE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android18GraphicBufferAllocE0_NS_20BnGraphicBufferAllocE");
        if(!new_ctor_str) new_ctor_str = (ctor_new_str_t)dlsym(h, "_ZTCN7android18GraphicBufferAllocE8_NS_7BBinderE");
    }
};
static Resolvers g_resolvers;
extern "C" void _ZN7android13GraphicBufferC1Ejjij(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage) asm("_ZN7android13GraphicBufferC1Ejjij");
extern "C" void _ZN7android13GraphicBufferC1Ejjij(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage){ g_resolvers.resolve(); if(g_resolvers.new_ctor_str){ g_resolvers.new_ctor_str(_this,inWidth,inHeight,inFormat,inUsage,std::string("")); return;} if(g_resolvers.new_ctor_cstr){ g_resolvers.new_ctor_cstr(_this,inWidth,inHeight,inFormat,inUsage,""); return;} if(g_resolvers.ctor_nativebuf){ g_resolvers.ctor_nativebuf(_this,nullptr,false); return;} std::memset(_this,0,sizeof(void*)); }
extern "C" void _ZN7android13GraphicBufferD1Ev(void* _this) asm("_ZN7android13GraphicBufferD1Ev");
extern "C" void _ZN7android13GraphicBufferD1Ev(void* _this){ g_resolvers.resolve(); if(g_resolvers.dtor){ g_resolvers.dtor(_this); return;} }
}
