#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <string>
#include <atomic>
#include <unistd.h>
#include <pthread.h>
#include <vector>
#include <algorithm>
#include <memory>
#include <mutex>
#include <typeinfo>
#include <functional>
#include <cstdio>
#include <cstdarg>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>


namespace android {

struct String8 {
    char* mString;
    String8() : mString(nullptr) {}
    String8(const char* s) {
        if (s) {
            mString = strdup(s);
        } else {
            mString = nullptr;
        }
    }
    ~String8() { free(mString); }
    String8(const String8& other) : mString(other.mString ? strdup(other.mString) : nullptr) {}
    String8& operator=(const String8& other) {
        if (this != &other) {
            free(mString);
            mString = other.mString ? strdup(other.mString) : nullptr;
        }
        return *this;
    }
    const char* string() const { return mString ? mString : ""; }
    size_t length() const { return mString ? strlen(mString) : 0; }
    void append(const char* s) {
        if (!s) return;
        size_t len = length();
        size_t slen = strlen(s);
        char* newStr = (char*)realloc(mString, len + slen + 1);
        if (newStr) {
            memcpy(newStr + len, s, slen + 1);
            mString = newStr;
        }
    }
    void setTo(const char* s) {
        free(mString);
        mString = s ? strdup(s) : nullptr;
    }
    void appendFormat(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        char buf[1024];
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        append(buf);
    }
};

struct String16 {
    uint16_t* mString;
    String16() : mString(nullptr) {}
    String16(const char* s) {
        if (s) {
            size_t len = strlen(s);
            mString = (uint16_t*)malloc((len + 1) * sizeof(uint16_t));
            if (mString) {
                for (size_t i = 0; i < len; ++i) mString[i] = s[i];
                mString[len] = 0;
            }
        } else {
            mString = nullptr;
        }
    }
    ~String16() { free(mString); }
    size_t size() const { return mString ? wcslen((wchar_t*)mString) : 0; }
};

struct Parcel {
    uint8_t* mData;
    size_t mDataSize;
    size_t mDataPos;
    Parcel() : mData(nullptr), mDataSize(0), mDataPos(0) {}
    ~Parcel() { free(mData); }
    void write(const void* data, size_t len) {
        if (mDataPos + len > mDataSize) {
            mDataSize = mDataPos + len + 1024;
            mData = (uint8_t*)realloc(mData, mDataSize);
        }
        memcpy(mData + mDataPos, data, len);
        mDataPos += len;
    }
    void writeInt32(int32_t val) { write(&val, sizeof(val)); }
    void writeInt64(int64_t val) { write(&val, sizeof(val)); }
    void writeUint32(uint32_t val) { write(&val, sizeof(val)); }
    void writeUint64(uint64_t val) { write(&val, sizeof(val)); }
    void writeString8(const String8& str) {
        uint32_t len = str.length();
        writeUint32(len);
        if (len) write(str.string(), len);
    }
    void writeNativeHandle(const native_handle* handle) {
        if (handle) {
            writeInt32(handle->numFds);
            writeInt32(handle->numInts);
            write(handle->data, sizeof(int) * (handle->numFds + handle->numInts));
        } else {
            writeInt32(0);
            writeInt32(0);
        }
    }
    void writeStrongBinder(const sp<IBinder>& binder) {
        
        writeInt32(binder != nullptr ? 1 : 0);
    }
    void writeInterfaceToken(const String16& token) {
        uint32_t len = token.size();
        writeUint32(len);
        if (len) write(token.mString, len * sizeof(uint16_t));
    }
    void writeBool(bool val) { writeInt32(val ? 1 : 0); }
    void writeFloat(float val) { write(&val, sizeof(val)); }
    int32_t readInt32() const {
        int32_t val;
        memcpy(&val, mData + mDataPos, sizeof(val));
        const_cast<Parcel*>(this)->mDataPos += sizeof(val);
        return val;
    }
    int64_t readInt64() const {
        int64_t val;
        memcpy(&val, mData + mDataPos, sizeof(val));
        const_cast<Parcel*>(this)->mDataPos += sizeof(val);
        return val;
    }
    uint32_t readUint32() const { return (uint32_t)readInt32(); }
    uint64_t readUint64() const { return (uint64_t)readInt64(); }
    String8 readString8() const {
        uint32_t len = readUint32();
        String8 str;
        if (len) {
            char* buf = (char*)malloc(len + 1);
            memcpy(buf, mData + mDataPos, len);
            buf[len] = 0;
            str.setTo(buf);
            free(buf);
            const_cast<Parcel*>(this)->mDataPos += len;
        }
        return str;
    }
    native_handle* readNativeHandle() const {
        int numFds = readInt32();
        int numInts = readInt32();
        native_handle* h = native_handle_create(numFds, numInts);
        memcpy(h->data, mData + mDataPos, sizeof(int) * (numFds + numInts));
        const_cast<Parcel*>(this)->mDataPos += sizeof(int) * (numFds + numInts);
        return h;
    }
    sp<IBinder> readStrongBinder() const {
        
        readInt32();
        return nullptr;
    }
    bool readBool() const { return readInt32() != 0; }
    void readFloat(float* val) const {
        memcpy(val, mData + mDataPos, sizeof(float));
        const_cast<Parcel*>(this)->mDataPos += sizeof(float);
    }
    bool checkInterface(IBinder* binder) const { return true; } // Stub
};

struct Rect {
    int32_t left, top, right, bottom;
    Rect() : left(0), top(0), right(0), bottom(0) {}
    Rect(int32_t l, int32_t t, int32_t r, int32_t b) : left(l), top(t), right(r), bottom(b) {}
    void makeInvalid() { left = right = top = bottom = -1; }
    void offsetBy(int32_t dx, int32_t dy) { left += dx; right += dx; top += dy; bottom += dy; }
    void offsetTo(int32_t x, int32_t y) { right = x + (right - left); bottom = y + (bottom - top); left = x; top = y; }
    bool intersect(const Rect& other, Rect* out) const {
        if (out) *out = *this;
        
        return true;
    }
    bool operator<(const Rect& other) const {
        if (left < other.left) return true;
        if (left > other.left) return false;
        if (top < other.top) return true;
        return false;
    }
    Rect operator+(const Point& p) const { Rect r = *this; r.offsetBy(p.x, p.y); return r; }
    Rect operator-(const Point& p) const { Rect r = *this; r.offsetBy(-p.x, -p.y); return r; }
    void reduce(const Rect& other) { /* stub */ }
    Rect transform(uint32_t, int, int) const { return *this; } // Stub
    static const Rect EMPTY_RECT;
    static const Rect INVALID_RECT;
};

const Rect Rect::EMPTY_RECT = Rect(0,0,0,0);
const Rect Rect::INVALID_RECT = Rect(-1,-1,-1,-1);

struct Point {
    int32_t x, y;
};

struct Region {
    
    std::vector<Rect> mRects;
    Region() {}
    Region(const Rect& r) { mRects.push_back(r); }
    Region(const Region& other) : mRects(other.mRects) {}
    ~Region() {}
    Region& operator=(const Region& other) { mRects = other.mRects; return *this; }
    void clear() { mRects.clear(); }
    void set(const Rect& r) { mRects.clear(); mRects.push_back(r); }
    void set(int w, int h) { set(Rect(0,0,w,h)); }
    void set(uint32_t w, uint32_t h) { set(static_cast<int>(w), static_cast<int>(h)); }
    void orSelf(const Rect& r) { mRects.push_back(r); } // Simplified
    void orSelf(const Region& other) { mRects.insert(mRects.end(), other.mRects.begin(), other.mRects.end()); }
    void orSelf(const Region& other, int dx, int dy) { for (auto r : other.mRects) { r.offsetBy(dx, dy); orSelf(r); } }
    void andSelf(const Rect& r) { /* stub */ }
    void andSelf(const Region& other) { /* stub */ }
    void andSelf(const Region& other, int dx, int dy) { /* stub */ }
    void xorSelf(const Rect& r) { /* stub */ }
    void xorSelf(const Region& other) { /* stub */ }
    void xorSelf(const Region& other, int dx, int dy) { /* stub */ }
    void subtractSelf(const Rect& r) { /* stub */ }
    void subtractSelf(const Region& other) { /* stub */ }
    void subtractSelf(const Region& other, int dx, int dy) { /* stub */ }
    void translateSelf(int dx, int dy) { for (auto& r : mRects) r.offsetBy(dx, dy); }
    void makeBoundsSelf() { /* stub */ }
    void addRectUnchecked(int l, int t, int r, int b) { mRects.push_back(Rect(l,t,r,b)); }
    static void boolean_operation(int op, Region& out, const Region& a, const Rect& b) { /* stub */ out = a; }
    static void boolean_operation(int op, Region& out, const Region& a, const Rect& b, int dx, int dy) { /* stub */ out = a; }
    static void boolean_operation(int op, Region& out, const Region& a, const Region& b) { /* stub */ out = a; }
    static void boolean_operation(int op, Region& out, const Region& a, const Region& b, int dx, int dy) { /* stub */ out = a; }
    static Region createTJunctionFreeRegion(const Region& r) { return r; }
    void operationSelf(const Rect& r, int op) { /* stub */ }
    void operationSelf(const Region& other, int op) { /* stub */ }
    void operationSelf(const Region& other, int op1, int op2, int op3) { /* stub */ }
    static void translate(Region& out, const Region& in, int dx, int dy) { out = in; out.translateSelf(dx, dy); }
    static void translate(Region& out, int dx, int dy) { out.translateSelf(dx, dy); }
    void unflatten(const void* data, size_t len) { /* stub */ }
    Region mergeExclusive(const Rect& r) const { Region res = *this; res.orSelf(r); return res; }
    Region mergeExclusive(const Region& other) const { Region res = *this; res.orSelf(other); return res; }
    Region mergeExclusive(const Region& other, int dx, int dy) const { Region res = *this; res.orSelf(other, dx, dy); return res; }
    size_t getFlattenedSize() const { return mRects.size() * sizeof(Rect); }
    bool isTriviallyEqual(const Region& other) const { return mRects == other.mRects; }
    const Rect* begin() const { return mRects.data(); }
    const Rect* end() const { return mRects.data() + mRects.size(); }
    String8 dump(const char* what, uint32_t flags) const { return String8("dump"); }
    void dump(String8& out, const char* what, uint32_t flags) const { out.append("dump"); }
    bool contains(const Point& p) const { return false; } // Stub
    bool contains(int x, int y) const { return contains(Point{x,y}); }
    uint32_t* getArray(uint32_t* count) const { if (count) *count = mRects.size(); return nullptr; } // Stub
    Region merge(const Rect& r) const { Region res = *this; res.orSelf(r); return res; }
    Region merge(const Region& other) const { Region res = *this; res.orSelf(other); return res; }
    Region merge(const Region& other, int dx, int dy) const { Region res = *this; res.orSelf(other, dx, dy); return res; }
    int flatten(void* buffer, size_t len) const { /* stub */ return 0; }
    Region subtract(const Rect& r) const { return *this; }
    Region subtract(const Region& other) const { return *this; }
    Region subtract(const Region& other, int dx, int dy) const { return *this; }
    Region intersect(const Rect& r) const { return *this; }
    Region intersect(const Region& other) const { return *this; }
    Region intersect(const Region& other, int dx, int dy) const { return *this; }
    Region operation(const Rect& r, int op) const { return *this; }
    Region operation(const Region& other, int op) const { return *this; }
    Region operation(const Region& other, int op1, int op2, int op3) const { return *this; }
    Region translate(int dx, int dy) const { Region res = *this; res.translateSelf(dx, dy); return res; }
    static const Region INVALID_REGION;
    struct rasterizer {
        virtual ~rasterizer() {}
        virtual void operator()(const Rect&) = 0;
        virtual void flushSpan() = 0;
    };
};

const Region Region::INVALID_REGION = Region(Rect::INVALID_RECT);

struct VectorImpl {
    void* mArray;
    size_t mCount;
    size_t mCapacity;
    uint32_t mItemSize;
    uint32_t mFlags;
    VectorImpl(size_t itemSize, uint32_t flags) : mArray(nullptr), mCount(0), mCapacity(0), mItemSize(itemSize), mFlags(flags) {}
    VectorImpl(const VectorImpl& other) : mArray(nullptr), mCount(0), mCapacity(0), mItemSize(other.mItemSize), mFlags(other.mFlags) { appendVector(other); }
    ~VectorImpl() { free(mArray); }
    VectorImpl& operator=(const VectorImpl& other) { clear(); appendVector(other); return *this; }
    void* editArrayImpl() { return mArray; }
    void resize(size_t newCount) { /* stub */ mCount = newCount; }
    void appendVector(const VectorImpl& other) { /* stub */ }
    void add(const void* item) { /* stub */ }
    void clear() { mCount = 0; }
    void insertAt(const void* item, size_t index, size_t num) { /* stub */ }
};

template <typename T>
struct Vector : public VectorImpl {
    Vector() : VectorImpl(sizeof(T), 0) {}
    void do_destroy(void* array, size_t count) const { /* stub */ }
    void do_construct(void* array, size_t count) const { /* stub */ }
    void do_move_forward(void* dst, const void* src, size_t count) const { memmove(dst, src, count * sizeof(T)); }
    void do_move_backward(void* dst, const void* src, size_t count) const { memmove(dst, src, count * sizeof(T)); }
    void do_copy(void* dst, const void* src, size_t count) const { memcpy(dst, src, count * sizeof(T)); }
    void do_splat(void* dst, const void* item, size_t count) const { /* stub */ }
};

template <typename T>
struct SortedVector : public Vector<T> {
    void do_compare(const void* a, const void* b) const { /* stub */ }
};

struct FrameStats {
    void unflatten(const void*, size_t) {}
    bool isFixedSize() const { return false; }
    size_t getFlattenedSize() const { return 0; }
    int flatten(void*, size_t) const { return 0; }
};

struct BufferItem {

    int getFdCount() const { return 0; }
    size_t getFlattenedSize() const { return 0; }
    int flatten(void*&, size_t&, int*&, size_t&) const { return 0; }
};

struct Fence {
    int mFd;
    Fence() : mFd(-1) {}
    Fence(int fd) : mFd(fd) {}
    ~Fence() { if (mFd >= 0) close(mFd); }
    int dup() const { return ::dup(mFd); }
    int getFdCount() const { return 1; }
    size_t getFlattenedSize() const { return sizeof(int); }
    int flatten(void*&, size_t&, int*&, size_t&) const { return 0; }
    void unflatten(const void*&, size_t&, const int*&, size_t&) { /* stub */ }
    int64_t getSignalTime() const { return 0; }
};

struct GraphicBuffer : public RefBase {
    
    uint32_t width, height;
    int32_t format;
    uint32_t usage, stride;
    native_handle* handle;
    GraphicBuffer() : width(0), height(0), format(0), usage(0), stride(0), handle(nullptr) {}
    GraphicBuffer(uint32_t w, uint32_t h, int32_t f, uint32_t u) : width(w), height(h), format(f), usage(u), stride(0), handle(nullptr) {}
    GraphicBuffer(uint32_t w, uint32_t h, int32_t f, uint32_t u, uint32_t s, native_handle* h, bool keep) : width(w), height(h), format(f), usage(u), stride(s), handle(h) {}
    GraphicBuffer(ANativeWindowBuffer* buf, bool keep) { /* stub */ }
    ~GraphicBuffer() { if (handle) native_handle_delete(handle); }
    int reallocate(uint32_t w, uint32_t h, int32_t f, uint32_t u) { return 0; }
    bool needsReallocation(uint32_t w, uint32_t h, int32_t f, uint32_t u) { return true; }
    void free_handle() { if (handle) native_handle_delete(handle); handle = nullptr; }
    int unlockAsync(int* out) { return 0; }
    int lockAsyncYCbCr(uint32_t usage, android_ycbcr* ycbcr, int fenceFd) { return 0; }
    int lockAsyncYCbCr(uint32_t usage, const Rect& rect, android_ycbcr* ycbcr, int fenceFd) { return 0; }
    int lock(uint32_t usage, void** vaddr) { return 0; }
    int lock(uint32_t usage, const Rect& rect, void** vaddr) { return 0; }
    int unlock() { return 0; }
    int initSize(uint32_t w, uint32_t h, int32_t f, uint32_t u) { return 0; }
    int lockAsync(uint32_t usage, void** vaddr, int fenceFd) { return 0; }
    int lockAsync(uint32_t usage, const Rect& rect, void** vaddr, int fenceFd) { return 0; }
    int lockYCbCr(uint32_t usage, android_ycbcr* ycbcr) { return 0; }
    int lockYCbCr(uint32_t usage, const Rect& rect, android_ycbcr* ycbcr) { return 0; }
    void unflatten(const void*&, size_t&, const int*&, size_t&) { /* stub */ }
    int getFdCount() const { return 0; }
    const ANativeWindowBuffer* getNativeBuffer() const { return nullptr; }
    size_t getFlattenedSize() const { return 0; }
    int flatten(void*&, size_t&, int*&, size_t&) const { return 0; }
    int initCheck() const { return 0; }
    static void dumpAllocationsToSystemLog() {}
};

struct GraphicBufferMapper {
    
    static GraphicBufferMapper& get() { static GraphicBufferMapper instance; return instance; }
    int unlockAsync(const native_handle* handle, int* out) { return 0; }
    int lockAsyncYCbCr(const native_handle* handle, uint32_t usage, const Rect& rect, android_ycbcr* ycbcr, int fenceFd) { return 0; }
    int registerBuffer(const native_handle* handle) { return 0; }
    int unregisterBuffer(const native_handle* handle) { return 0; }
    int lock(const native_handle* handle, uint32_t usage, const Rect& rect, void** vaddr) { return 0; }
    int unlock(const native_handle* handle) { return 0; }
    int lockAsync(const native_handle* handle, uint32_t usage, const Rect& rect, void** vaddr, int fenceFd) { return 0; }
    int lockYCbCr(const native_handle* handle, uint32_t usage, const Rect& rect, android_ycbcr* ycbcr) { return 0; }
};

struct GraphicBufferAllocator {
    
    static GraphicBufferAllocator& get() { static GraphicBufferAllocator instance; return instance; }
    static void dumpToSystemLog() {}
    int free(const native_handle* handle) { return 0; }
    int alloc(uint32_t w, uint32_t h, int32_t f, uint32_t u, const native_handle** outHandle, uint32_t* outStride) { return 0; }
};

struct RefBase {
    virtual ~RefBase() {}
    virtual void onFirstRef() {}
    virtual void onLastWeakRef(const void*) {}
    virtual void onLastStrongRef(const void*) {}
    virtual bool onIncStrongAttempted(uint32_t, const void*) { return true; }
    RefBase() {}
    int decStrong(const void*) const { return 0; }
    int incStrong(const void*) const { return 0; }
    struct weakref_type {
        void trackMe(bool, bool) {}
        void printRefs() const {}
    };
    weakref_type* getWeakRefs() const { return nullptr; }
    int getStrongCount() const { return 1; }
};

struct IInterface : public RefBase {
    virtual ~IInterface() {}
};

struct IBinder : public RefBase {
    virtual ~IBinder() {}
    virtual BBinder* localBinder() { return nullptr; }
    virtual BpBinder* remoteBinder() { return nullptr; }
    virtual sp<IInterface> queryLocalInterface(const String16&) { return nullptr; }
    static bool checkSubclass(const void*) { return true; }
    struct DeathRecipient {
        virtual ~DeathRecipient() {}
    };
};

struct BBinder : public IBinder {
    virtual int onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) { return 0; }
    virtual int pingBinder() { return 0; }
    virtual bool isBinderAlive() const { return true; }
    virtual String16 getInterfaceDescriptor() const { return String16(""); }
    virtual int transact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) { return 0; }
    virtual status_t linkToDeath(const sp<IBinder::DeathRecipient>&, void*, uint32_t) { return 0; }
    virtual status_t unlinkToDeath(const wp<IBinder::DeathRecipient>&, void*, uint32_t, wp<IBinder::DeathRecipient>*) { return 0; }
    virtual BBinder* localBinder() { return this; }
    virtual void attachObject(const void*, void*, void*, void (*)(const void*, void*, void*)) {}
    virtual void* findObject(const void*) const { return nullptr; }
    virtual void detachObject(const void*) {}
    virtual int dump(int fd, const Vector<String16>& args) { return 0; }
    BBinder() {}
};

struct BpRefBase : public RefBase {
    BpRefBase(const sp<IBinder>& o) {}
    virtual ~BpRefBase() {}
    virtual void onFirstRef() {}
    virtual void onLastStrongRef(const void*) {}
    virtual bool onIncStrongAttempted(uint32_t, const void*) { return true; }
};

template <typename INTERFACE>
struct BnInterface : public INTERFACE, public BBinder {
    virtual String16 getInterfaceDescriptor() const { return INTERFACE::getInterfaceDescriptor(); }
};

template <typename INTERFACE>
struct BpInterface : public INTERFACE, public BpRefBase {
    BpInterface(const sp<IBinder>& remote) : BpRefBase(remote) {}
};

struct IGraphicBufferConsumer : public IInterface {
    static const String16 descriptor;
    virtual String16 getInterfaceDescriptor() const { return descriptor; }
    IGraphicBufferConsumer() {}
    virtual ~IGraphicBufferConsumer() {}
};

const String16 IGraphicBufferConsumer::descriptor("android.IGraphicBufferConsumer");

struct BnGraphicBufferConsumer : public BnInterface<IGraphicBufferConsumer> {
    virtual int onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) { return 0; }
};

struct BpGraphicBufferConsumer : public BpInterface<IGraphicBufferConsumer> {
    BpGraphicBufferConsumer(const sp<IBinder>& remote) : BpInterface<IGraphicBufferConsumer>(remote) {}
    virtual ~BpGraphicBufferConsumer() {}
};

struct IDumpTunnel : public IInterface {
    static const String16 descriptor;
    virtual String16 getInterfaceDescriptor() const { return descriptor; }
};

const String16 IDumpTunnel::descriptor("android.IDumpTunnel");

struct BnDumpTunnel : public BnInterface<IDumpTunnel> {
    virtual int onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags) { return 0; }
};

struct BpDumpTunnel : public BpInterface<IDumpTunnel> {
    BpDumpTunnel(const sp<IBinder>& remote) : BpInterface<IDumpTunnel>(remote) {}
    virtual ~BpDumpTunnel() {}
};

struct OccupancyTracker {
    struct Segment {
        // Stub
    };
};

struct HdrCapabilities : public Parcelable {
    // Stub
    int writeToParcel(Parcel* p) const { return 0; }
    int readFromParcel(const Parcel* p) { return 0; }
};

struct ComposerState {
    // Stub
};

struct DisplayState {
    // Stub
};

struct ComposerService {
  
    static ComposerService& get() { static ComposerService instance; return instance; }
};

struct Composer {
    
    static Composer& get() { static Composer instance; return instance; }
    sp<IBinder> createDisplay(const String8&) { return nullptr; }
    void destroyDisplay(const sp<IBinder>&) {}
    void setDisplaySize(const sp<IBinder>&, uint32_t, uint32_t) {}
    sp<IBinder> getBuiltInDisplay(int) { return nullptr; }
    void setDisplaySurface(const sp<IBinder>&, const sp<IGraphicBufferProducer>&) {}
    void setBlurMaskSurface(const sp<SurfaceComposerClient>&, const sp<IBinder>&, const sp<IBinder>&) {}
    void setBlurMaskSampling(const sp<SurfaceComposerClient>&, const sp<IBinder>&, uint32_t) {}
    void deferTransactionUntil(const sp<SurfaceComposerClient>&, const sp<IBinder>&, const sp<IBinder>&, uint64_t) {}
    void setDisplayLayerStack(const sp<IBinder>&, uint32_t) {}
    void setDisplayProjection(const sp<IBinder>&, uint32_t, const Rect&, const Rect&) {}
    void setOverrideScalingMode(const sp<SurfaceComposerClient>&, const sp<IBinder>&, int) {}
    void setTransparentRegionHint(const sp<SurfaceComposerClient>&, const sp<IBinder>&, const Region&) {}
    void openGlobalTransactionImpl() {}
    void setBlurMaskAlphaThreshold(const sp<SurfaceComposerClient>&, const sp<IBinder>&, float) {}
    void closeGlobalTransactionImpl(bool) {}
    void setAnimationTransactionImpl() {}
    void setGeometryAppliesWithResize(const sp<SurfaceComposerClient>&, const sp<IBinder>&) {}
    void setBlur(const sp<SurfaceComposerClient>&, const sp<IBinder>&, float) {}
    void setCrop(const sp<SurfaceComposerClient>&, const sp<IBinder>&, const Rect&) {}
    void setSize(const sp<SurfaceComposerClient>&, const sp<IBinder>&, uint32_t, uint32_t) {}
    void setAlpha(const sp<SurfaceComposerClient>&, const sp<IBinder>&, float) {}
    void setColor(const sp<SurfaceComposerClient>&, const sp<IBinder>&, uint32_t) {}
    void setFlags(const sp<SurfaceComposerClient>&, const sp<IBinder>&, uint32_t, uint32_t) {}
    void setLayer(const sp<SurfaceComposerClient>&, const sp<IBinder>&, uint32_t) {}
    void setMatrix(const sp<SurfaceComposerClient>&, const sp<IBinder>&, float, float, float, float) {}
    void setPosition(const sp<SurfaceComposerClient>&, const sp<IBinder>&, float, float) {}
    ComposerState getLayerStateLocked(const sp<SurfaceComposerClient>&, const sp<IBinder>&) { return ComposerState{}; }
    DisplayState getDisplayStateLocked(const sp<IBinder>&) { return DisplayState{}; }
};

struct SurfaceComposerClient : public RefBase {
    sp<IBinder> connection() const { return nullptr; }
    status_t initCheck() const { return 0; }
    virtual ~SurfaceComposerClient() {}
};

struct SyncFeatures {
    SyncFeatures() {}
    String8 toString() const { return String8(""); }
};

struct BufferQueueCore {
    // Stub
};

struct BufferQueueDebug {
    // Stub
    virtual ~BufferQueueDebug() {}
};

struct BufferQueueMonitor {
    void dump(String8&, const char*) {}
    virtual ~BufferQueueMonitor() {}
};

struct GuiExtMonitor {
    // Stub
};

struct DumpTunnelHelper {
    DumpTunnelHelper() {}
    virtual ~DumpTunnelHelper() {}
};

struct FreeModeDevice {
    void setScenario(const uint32_t&) {}
    void query(bool*) {}
    FreeModeDevice() {}
    virtual ~FreeModeDevice() {}
};

struct PerfServiceController {
    int getCapInfo() { return 0; }
    void parseConfig(char*, std::vector<int>&) {}
    void assignConfig(const int&, const int&, const int&) {}
    void setCPUScenario(const bool&) {}
    PerfServiceController() {}
    virtual ~PerfServiceController() {}
};

struct RefreshRateControl {
    void setScenario(int, bool) {}
    RefreshRateControl() {}
    virtual ~RefreshRateControl() {}
};

struct GraphicBufferUtil {
    static void drawLine(const sp<GraphicBuffer>&, uint8_t, int, int, int) {}
};

struct CallStack {
    CallStack(const char*, int) {}
    ~CallStack() {}
};

template <typename T>
struct Singleton {
    static T& getInstance() { static T instance; return instance; }
    static bool hasInstance() { return true; }
    static pthread_mutex_t sLock;
    static T* sInstance;
    Singleton() {}
    ~Singleton() {}
};

template <typename T>
pthread_mutex_t Singleton<T>::sLock = PTHREAD_MUTEX_INITIALIZER;

template <typename T>
T* Singleton<T>::sInstance = nullptr;



} 
static void* open_libgui() {
    static std::atomic<void*> handle{nullptr};
    void* h = handle.load(std::memory_order_acquire);
    if (h) return h;
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&m);
    h = handle.load(std::memory_order_relaxed);
    if (h) {
        pthread_mutex_unlock(&m);
        return h;
    }
    h = dlopen("/system/lib/libgui.so", RTLD_LAZY | RTLD_LOCAL);
    if (!h) h = dlopen("/system/lib64/libgui.so", RTLD_LAZY | RTLD_LOCAL);
    if (!h) h = dlopen("libgui.so", RTLD_LAZY | RTLD_LOCAL);
    handle.store(h, std::memory_order_release);
    pthread_mutex_unlock(&m);
    return h;
}

static void* open_libui() {
    static std::atomic<void*> handle{nullptr};
    void* h = handle.load(std::memory_order_acquire);
    if (h) return h;
    static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&m);
    h = handle.load(std::memory_order_relaxed);
    if (h) {
        pthread_mutex_unlock(&m);
        return h;
    }
    h = dlopen("/system/lib/libui.so", RTLD_LAZY | RTLD_LOCAL);
    if (!h) h = dlopen("/system/lib64/libui.so", RTLD_LAZY | RTLD_LOCAL);
    if (!h) h = dlopen("libui.so", RTLD_LAZY | RTLD_LOCAL);
    handle.store(h, std::memory_order_release);
    pthread_mutex_unlock(&m);
    return h;
}

namespace shim {

struct GlobalResolvers {
    std::mutex mutex;
    bool resolved = false;
    void* libgui = nullptr;
    void* libui = nullptr;
    
    decltype(&android::IGraphicBufferConsumer::~IGraphicBufferConsumer) IGraphicBufferConsumer_dtor = nullptr;
    decltype(&android::BnGraphicBufferConsumer::onTransact) BnGraphicBufferConsumer_onTransact = nullptr;
    decltype(&android::BpGraphicBufferConsumer::~BpGraphicBufferConsumer) BpGraphicBufferConsumer_dtor = nullptr;
    decltype(&android::Fence::unflatten) Fence_unflatten = nullptr;
    decltype(&android::Fence::Fence) Fence_ctor = nullptr;
    decltype(&android::Fence::~Fence) Fence_dtor = nullptr;
    decltype(&android::Parcel::writeInt32) Parcel_writeInt32 = nullptr;
    decltype(&android::Parcel::writeInt64) Parcel_writeInt64 = nullptr;
    decltype(&android::Parcel::writeUint32) Parcel_writeUint32 = nullptr;
    decltype(&android::Parcel::writeUint64) Parcel_writeUint64 = nullptr;
    decltype(&android::Parcel::writeString8) Parcel_writeString8 = nullptr;
    decltype(&android::Parcel::writeNativeHandle) Parcel_writeNativeHandle = nullptr;
    decltype(&android::Parcel::writeStrongBinder) Parcel_writeStrongBinder = nullptr;
    decltype(&android::Parcel::writeInterfaceToken) Parcel_writeInterfaceToken = nullptr;
    decltype(&android::Parcel::write) Parcel_write_Flattenable = nullptr;
    decltype(&android::Parcel::Parcel) Parcel_ctor = nullptr;
    decltype(&android::Parcel::~Parcel) Parcel_dtor = nullptr;
    decltype(&android::BBinder::onTransact) BBinder_onTransact = nullptr;
    decltype(&android::BBinder::pingBinder) BBinder_pingBinder = nullptr;
    decltype(&android::BBinder::linkToDeath) BBinder_linkToDeath = nullptr;
    decltype(&android::BBinder::localBinder) BBinder_localBinder = nullptr;
    decltype(&android::BBinder::attachObject) BBinder_attachObject = nullptr;
    decltype(&android::BBinder::detachObject) BBinder_detachObject = nullptr;
    decltype(&android::BBinder::unlinkToDeath) BBinder_unlinkToDeath = nullptr;
    decltype(&android::BBinder::dump) BBinder_dump = nullptr;
    decltype(&android::BBinder::transact) BBinder_transact = nullptr;
    decltype(&android::BBinder::~BBinder) BBinder_dtor = nullptr;
    decltype(&android::IBinder::localBinder) IBinder_localBinder = nullptr;
    decltype(&android::IBinder::remoteBinder) IBinder_remoteBinder = nullptr;
    decltype(&android::IBinder::queryLocalInterface) IBinder_queryLocalInterface = nullptr;
    decltype(&android::IBinder::~IBinder) IBinder_dtor = nullptr;
    decltype(&android::RefBase::onFirstRef) RefBase_onFirstRef = nullptr;
    decltype(&android::RefBase::onLastWeakRef) RefBase_onLastWeakRef = nullptr;
    decltype(&android::RefBase::onLastStrongRef) RefBase_onLastStrongRef = nullptr;
    decltype(&android::RefBase::onIncStrongAttempted) RefBase_onIncStrongAttempted = nullptr;
    decltype(&android::RefBase::RefBase) RefBase_ctor = nullptr;
    decltype(&android::RefBase::~RefBase) RefBase_dtor = nullptr;
    decltype(&android::String8::setTo) String8_setTo = nullptr;
    decltype(&android::String8::String8) String8_ctor_cstr = nullptr;
    decltype(&android::String8::~String8) String8_dtor = nullptr;
    decltype(&android::String16::String16) String16_ctor = nullptr;
    decltype(&android::String16::~String16) String16_dtor = nullptr;
    decltype(&android::BpRefBase::onFirstRef) BpRefBase_onFirstRef = nullptr;
    decltype(&android::BpRefBase::onLastStrongRef) BpRefBase_onLastStrongRef = nullptr;
    decltype(&android::BpRefBase::onIncStrongAttempted) BpRefBase_onIncStrongAttempted = nullptr;
    decltype(&android::BpRefBase::BpRefBase) BpRefBase_ctor = nullptr;
    decltype(&android::BpRefBase::~BpRefBase) BpRefBase_dtor = nullptr;
    decltype(&android::BufferItem::getFdCount) BufferItem_getFdCount = nullptr;
    decltype(&android::BufferItem::getFlattenedSize) BufferItem_getFlattenedSize = nullptr;
    decltype(&android::BufferItem::flatten) BufferItem_flatten = nullptr;
    decltype(&android::GraphicBuffer::getFdCount) GraphicBuffer_getFdCount = nullptr;
    decltype(&android::GraphicBuffer::getFlattenedSize) GraphicBuffer_getFlattenedSize = nullptr;
    decltype(&android::GraphicBuffer::flatten) GraphicBuffer_flatten = nullptr;
    decltype(&android::IGraphicBufferConsumer::getInterfaceDescriptor) IGraphicBufferConsumer_getInterfaceDescriptor = nullptr;
    decltype(&android::Fence::getFdCount) Fence_getFdCount = nullptr;
    decltype(&android::Fence::getFlattenedSize) Fence_getFlattenedSize = nullptr;
    decltype(&android::Fence::flatten) Fence_flatten = nullptr;
    decltype(&android::Parcel::readUint32) Parcel_readUint32 = nullptr;
    decltype(&android::Parcel::readUint64) Parcel_readUint64 = nullptr;
    decltype(&android::Parcel::readString8) Parcel_readString8 = nullptr;
    decltype(&android::Parcel::checkInterface) Parcel_checkInterface = nullptr;
    decltype(&android::Parcel::readNativeHandle) Parcel_readNativeHandle = nullptr;
    decltype(&android::Parcel::readStrongBinder) Parcel_readStrongBinder = nullptr;
    decltype(&android::Parcel::read) Parcel_read_Flattenable = nullptr;
    decltype(&android::Parcel::readInt32) Parcel_readInt32 = nullptr;
    decltype(&android::Parcel::readInt64) Parcel_readInt64 = nullptr;
    decltype(&android::BBinder::findObject) BBinder_findObject = nullptr;
    decltype(&android::BBinder::isBinderAlive) BBinder_isBinderAlive = nullptr;
    decltype(&android::BBinder::getInterfaceDescriptor) BBinder_getInterfaceDescriptor = nullptr;
    decltype(&android::IBinder::checkSubclass) IBinder_checkSubclass = nullptr;
    decltype(&android::RefBase::decStrong) RefBase_decStrong = nullptr;
    decltype(&android::RefBase::incStrong) RefBase_incStrong = nullptr;
    decltype(&android::String16::size) String16_size = nullptr;
    
    decltype(&android::Fence::Fence) Fence_ctor_int = nullptr;
    decltype(&android::Fence::getSignalTime) Fence_getSignalTime = nullptr;
    decltype(&android::Fence::dup) Fence_dup = nullptr;
    decltype(&android::FrameStats::unflatten) FrameStats_unflatten = nullptr;
    decltype(&android::VectorImpl::editArrayImpl) VectorImpl_editArrayImpl = nullptr;
    decltype(&android::VectorImpl::resize) VectorImpl_resize = nullptr;
    decltype(&android::FrameStats::isFixedSize) FrameStats_isFixedSize = nullptr;
    decltype(&android::FrameStats::getFlattenedSize) FrameStats_getFlattenedSize = nullptr;
    decltype(&android::FrameStats::flatten) FrameStats_flatten = nullptr;
    decltype(&android::GraphicBuffer::reallocate) GraphicBuffer_reallocate = nullptr;
    decltype(&android::GraphicBuffer::free_handle) GraphicBuffer_free_handle = nullptr;
    decltype(&android::GraphicBuffer::unlockAsync) GraphicBuffer_unlockAsync = nullptr;
    decltype(&android::GraphicBuffer::lockAsyncYCbCr) GraphicBuffer_lockAsyncYCbCr = nullptr;
    decltype(&android::GraphicBuffer::lockAsyncYCbCr) GraphicBuffer_lockAsyncYCbCr_rect = nullptr;
    decltype(&android::GraphicBuffer::needsReallocation) GraphicBuffer_needsReallocation = nullptr;
    decltype(&android::GraphicBuffer::dumpAllocationsToSystemLog) GraphicBuffer_dumpAllocationsToSystemLog = nullptr;
    decltype(&android::GraphicBuffer::lock) GraphicBuffer_lock = nullptr;
    decltype(&android::GraphicBuffer::lock) GraphicBuffer_lock_rect = nullptr;
    decltype(&android::GraphicBuffer::unlock) GraphicBuffer_unlock = nullptr;
    decltype(&android::GraphicBuffer::initSize) GraphicBuffer_initSize = nullptr;
    decltype(&android::GraphicBuffer::lockAsync) GraphicBuffer_lockAsync = nullptr;
    decltype(&android::GraphicBuffer::lockAsync) GraphicBuffer_lockAsync_rect = nullptr;
    decltype(&android::GraphicBuffer::lockYCbCr) GraphicBuffer_lockYCbCr = nullptr;
    decltype(&android::GraphicBuffer::lockYCbCr) GraphicBuffer_lockYCbCr_rect = nullptr;
    decltype(&android::GraphicBuffer::unflatten) GraphicBuffer_unflatten = nullptr;
    decltype(&android::GraphicBuffer::GraphicBuffer) GraphicBuffer_ctor_anw = nullptr;
    decltype(&android::GraphicBuffer::GraphicBuffer) GraphicBuffer_ctor_whfu = nullptr;
    decltype(&android::GraphicBuffer::GraphicBuffer) GraphicBuffer_ctor_whfusnh = nullptr;
    decltype(&android::GraphicBuffer::GraphicBuffer) GraphicBuffer_ctor = nullptr;
    decltype(&android::GraphicBuffer::~GraphicBuffer) GraphicBuffer_dtor_ui = nullptr;
    decltype(&android::GraphicBufferMapper::unlockAsync) GraphicBufferMapper_unlockAsync = nullptr;
    decltype(&android::GraphicBufferMapper::lockAsyncYCbCr) GraphicBufferMapper_lockAsyncYCbCr = nullptr;
    decltype(&android::GraphicBufferMapper::registerBuffer) GraphicBufferMapper_registerBuffer = nullptr;
    decltype(&android::GraphicBufferMapper::unregisterBuffer) GraphicBufferMapper_unregisterBuffer = nullptr;
    decltype(&android::GraphicBufferMapper::lock) GraphicBufferMapper_lock = nullptr;
    decltype(&android::GraphicBufferMapper::unlock) GraphicBufferMapper_unlock = nullptr;
    decltype(&android::GraphicBufferMapper::lockAsync) GraphicBufferMapper_lockAsync = nullptr;
    decltype(&android::GraphicBufferMapper::lockYCbCr) GraphicBufferMapper_lockYCbCr = nullptr;
    decltype(&android::GraphicBufferMapper::GraphicBufferMapper) GraphicBufferMapper_ctor = nullptr;
    decltype(&android::GraphicBufferAllocator::dumpToSystemLog) GraphicBufferAllocator_dumpToSystemLog = nullptr;
    decltype(&android::GraphicBufferAllocator::free) GraphicBufferAllocator_free = nullptr;
    decltype(&android::GraphicBufferAllocator::alloc) GraphicBufferAllocator_alloc = nullptr;
    decltype(&android::GraphicBufferAllocator::GraphicBufferAllocator) GraphicBufferAllocator_ctor = nullptr;
    decltype(&android::GraphicBuffer::getNativeBuffer) GraphicBuffer_getNativeBuffer = nullptr;
    decltype(&android::GraphicBuffer::initCheck) GraphicBuffer_initCheck = nullptr;
    decltype(&android::RefBase::decStrong) RefBase_decStrong_ui = nullptr;
    decltype(&android::RefBase::incStrong) RefBase_incStrong_ui = nullptr;
  

    void resolve() {
        std::lock_guard<std::mutex> lock(mutex);
        if (resolved) return;
        resolved = true;
        libgui = open_libgui();
        libui = open_libui();
        if (libgui) {
            IGraphicBufferConsumer_dtor = (decltype(IGraphicBufferConsumer_dtor)) dlsym(libgui, "_ZN7android22IGraphicBufferConsumerD0Ev");
            if (!IGraphicBufferConsumer_dtor) IGraphicBufferConsumer_dtor = (decltype(IGraphicBufferConsumer_dtor)) dlsym(libgui, "_ZN7android22IGraphicBufferConsumerD1Ev");
        
        }
        if (libui) {
            
            Fence_ctor_int = (decltype(Fence_ctor_int)) dlsym(libui, "_ZN7android5FenceC1Ei");
            
        }
    }
};


static GlobalResolvers g_resolvers;



extern "C" void _ZN7android22IGraphicBufferConsumerD0Ev(android::IGraphicBufferConsumer* _this) {
    g_resolvers.resolve();
    if (g_resolvers.IGraphicBufferConsumer_dtor) g_resolvers.IGraphicBufferConsumer_dtor(_this);
}

extern "C" void _ZN7android22IGraphicBufferConsumerD1Ev(android::IGraphicBufferConsumer* _this) {
    g_resolvers.resolve();
    if (g_resolvers.IGraphicBufferConsumer_dtor) g_resolvers.IGraphicBufferConsumer_dtor(_this);
}

extern "C" void _ZN7android22IGraphicBufferConsumerD2Ev(android::IGraphicBufferConsumer* _this) {
    g_resolvers.resolve();
    if (g_resolvers.IGraphicBufferConsumer_dtor) g_resolvers.IGraphicBufferConsumer_dtor(_this);
}

extern "C" int _ZN7android23BnGraphicBufferConsumer10onTransactEjRKNS_6ParcelEPS1_j(android::BnGraphicBufferConsumer* _this, uint32_t code, const android::Parcel& data, android::Parcel* reply, uint32_t flags) {
    g_resolvers.resolve();
    if (g_resolvers.BnGraphicBufferConsumer_onTransact) return g_resolvers.BnGraphicBufferConsumer_onTransact(_this, code, data, reply, flags);
    return 0;
}

extern "C" void _ZN7android23BpGraphicBufferConsumerD0Ev(android::BpGraphicBufferConsumer* _this) {
    g_resolvers.resolve();
    if (g_resolvers.BpGraphicBufferConsumer_dtor) g_resolvers.BpGraphicBufferConsumer_dtor(_this);
}

extern "C" void _ZN7android23BpGraphicBufferConsumerD1Ev(android::BpGraphicBufferConsumer* _this) {
    g_resolvers.resolve();
    if (g_resolvers.BpGraphicBufferConsumer_dtor) g_resolvers.BpGraphicBufferConsumer_dtor(_this);
}

extern "C" void _ZN7android23BpGraphicBufferConsumerD2Ev(android::BpGraphicBufferConsumer* _this) {
    g_resolvers.resolve();
    if (g_resolvers.BpGraphicBufferConsumer_dtor) g_resolvers.BpGraphicBufferConsumer_dtor(_this);
}

extern "C" void _ZN7android5Fence9unflattenERPKvRmRPKiS4_(android::Fence* _this, const void*& buffer, size_t& size, const int*& fds, size_t& numFds) {
    g_resolvers.resolve();
    if (g_resolvers.Fence_unflatten) g_resolvers.Fence_unflatten(_this, buffer, size, fds, numFds);
}

extern "C" void _ZN7android5FenceC1Ev(android::Fence* _this) {
    g_resolvers.resolve();
    if (g_resolvers.Fence_ctor) g_resolvers.Fence_ctor(_this);
}

extern "C" void _ZN7android5FenceD1Ev(android::Fence* _this) {
    g_resolvers.resolve();
    if (g_resolvers.Fence_dtor) g_resolvers.Fence_dtor(_this);
}

extern "C" void _ZN7android5FenceD2Ev(android::Fence* _this) {
    g_resolvers.resolve();
    if (g_resolvers.Fence_dtor) g_resolvers.Fence_dtor(_this);
}


extern "C" void _ZN7android5FenceC1Ei(android::Fence* _this, int fd) {
    g_resolvers.resolve();
    if (g_resolvers.Fence_ctor_int) g_resolvers.Fence_ctor_int(_this, fd);
    else if (g_resolvers.Fence_ctor) g_resolvers.Fence_ctor(_this); 
}

extern "C" void _ZN7android5FenceC2Ei(android::Fence* _this, int fd) {
    _ZN7android5FenceC1Ei(_this, fd);
}


extern "C" void _ZN7android13GraphicBufferC1EjjijjP13native_handleb(android::GraphicBuffer* _this, uint32_t w, uint32_t h, int32_t f, uint32_t u, uint32_t s, native_handle* h, bool keep) {
    g_resolvers.resolve();
    
    if (g_resolvers.GraphicBuffer_ctor_whfusnh) g_resolvers.GraphicBuffer_ctor_whfusnh(_this, w, h, f, u, s, h, keep);
}

}
