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
using cfg_consumer_t = int (*)(void*, uint32, bool, uint32, uint32, uint32);
static cfg_consumer_t real_configDisplay = nullptr;
static void resolve_configDisplay(){
    if(real_configDisplay) return;
    void* hg = open_libgui();
    if(!hg) hg = open_libui();
    if(!hg) return;
    const char* names[] = {
        "_ZN7android21GuiExtClientConsumer13configDisplayEjbjjj",
        "_ZN7android20GuiExtClientConsumer13configDisplayEjbjjj",
        "_ZN7android20GuiExtClientConsumer13configDisplayEjjjj",
        "_ZN7android21GuiExtClientConsumer13configDisplayEjjjj",
        nullptr
    };
    for(const char** p = names; *p; ++p){
        void* s = dlsym(hg, *p);
        if(s){ real_configDisplay = reinterpret_cast<cfg_consumer_t>(s); return; }
    }
}
using fnptr_t = void(*)();
static fnptr_t try_resolve(const char* name){
    void* h = open_libgui();
    if(!h) h = open_libui();
    if(!h) return nullptr;
    void* s = dlsym(h, name);
    return reinterpret_cast<fnptr_t>(s);
}
extern "C" int _ZN7android21GuiExtClientConsumer13configDisplayEjbjjj(void* _this, uint32 a, bool b, uint32 c, uint32 d, uint32 e) asm("_ZN7android21GuiExtClientConsumer13configDisplayEjbjjj");
extern "C" int _ZN7android21GuiExtClientConsumer13configDisplayEjbjjj(void* _this, uint32 a, bool b, uint32 c, uint32 d, uint32 e){
    resolve_configDisplay();
    if(real_configDisplay){ return real_configDisplay(_this, a, b, c, d, e); }
    fnptr_t f = try_resolve("_ZN7android8Composer13setDisplayStateERKNS_2spINS_21SurfaceComposerClientEEEjbjjj");
    if(f) return reinterpret_cast<int(*)(void*,uint32,bool,uint32,uint32,uint32)>(f)(_this,a,b,c,d,e);
    return 0;
}
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE14getProcessNameEv() asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE14getProcessNameEv");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE14getProcessNameEv(){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE14getProcessNameEv"); if(f) f(); }
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE4dumpERNS_7String8EPKc(void* _this, void* s, const char* c) asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE4dumpERNS_7String8EPKc");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE4dumpERNS_7String8EPKc(void* _this, void* s, const char* c){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE4dumpERNS_7String8EPKc"); if(f) reinterpret_cast<void(*)(void*,void*,const char*)>(f)(_this,s,c); }
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE7monitorES3_(void* _this, void* a) asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE7monitorES3_");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE7monitorES3_(void* _this, void* a){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE7monitorES3_"); if(f) reinterpret_cast<void(*)(void*,void*)>(f)(_this,a); }
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE9unmonitorES3_(void* _this, void* a) asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE9unmonitorES3_");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE9unmonitorES3_(void* _this, void* a){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEE9unmonitorES3_"); if(f) reinterpret_cast<void(*)(void*,void*)>(f)(_this,a); }
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED0Ev(void* _this) asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED0Ev");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED0Ev(void* _this){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED0Ev"); if(f) reinterpret_cast<void(*)(void*)>(f)(_this); }
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED2Ev(void* _this) asm("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED2Ev");
extern "C" void _ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED2Ev(void* _this){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_14RefBaseMonitorEPNS_7RefBaseEED2Ev"); if(f) reinterpret_cast<void(*)(void*)>(f)(_this); }
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE14getProcessNameEv() asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE14getProcessNameEv");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE14getProcessNameEv(){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE14getProcessNameEv"); if(f) f(); }
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc(void* _this, void* s, const char* c) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc(void* _this, void* s, const char* c){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE4dumpERNS_7String8EPKc"); if(f) reinterpret_cast<void(*)(void*,void*,const char*)>(f)(_this,s,c); }
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4_(void* _this, void* a) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4_");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4_(void* _this, void* a){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE7monitorES4_"); if(f) reinterpret_cast<void(*)(void*,void*)>(f)(_this,a); }
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE9unmonitorES4_(void* _this, void* a) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE9unmonitorES4_");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE9unmonitorES4_(void* _this, void* a){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEE9unmonitorES4_"); if(f) reinterpret_cast<void(*)(void*,void*)>(f)(_this,a); }
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED0Ev(void* _this) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED0Ev");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED0Ev(void* _this){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED0Ev"); if(f) reinterpret_cast<void(*)(void*)>(f)(_this); }
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED2Ev(void* _this) asm("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED2Ev");
extern "C" void _ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED2Ev(void* _this){ fnptr_t f = try_resolve("_ZN7android13GuiExtMonitorINS_18BufferQueueMonitorENS_2wpINS_15BufferQueueCoreEEEED2Ev"); if(f) reinterpret_cast<void(*)(void*)>(f)(_this); }
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
    }
};
static Resolvers g_resolvers;
extern "C" void _ZN7android13GraphicBufferC1Ejjij(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage) asm("_ZN7android13GraphicBufferC1Ejjij");
extern "C" void _ZN7android13GraphicBufferC1Ejjij(void* _this, uint32 inWidth, uint32 inHeight, int32 inFormat, uint32 inUsage){ g_resolvers.resolve(); if(g_resolvers.new_ctor_str){ g_resolvers.new_ctor_str(_this,inWidth,inHeight,inFormat,inUsage,std::string("")); return;} if(g_resolvers.new_ctor_cstr){ g_resolvers.new_ctor_cstr(_this,inWidth,inHeight,inFormat,inUsage,""); return;} if(g_resolvers.ctor_nativebuf){ g_resolvers.ctor_nativebuf(_this,nullptr,false); return;} std::memset(_this,0,sizeof(void*)); }
extern "C" void _ZN7android13GraphicBufferD1Ev(void* _this) asm("_ZN7android13GraphicBufferD1Ev");
extern "C" void _ZN7android13GraphicBufferD1Ev(void* _this){ g_resolvers.resolve(); if(g_resolvers.dtor){ g_resolvers.dtor(_this); return;} }
}
