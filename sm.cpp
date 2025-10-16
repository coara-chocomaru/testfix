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
#include <wchar.h>

namespace android {

typedef int status_t;

struct String8 {
    char* mString;
    String8() : mString(nullptr) {}
    String8(const char* s) {
        if (s) mString = strdup(s); else mString = nullptr;
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
                for (size_t i = 0; i < len; ++i) mString[i] = (unsigned char)s[i];
                mString[len] = 0;
            }
        } else {
            mString = nullptr;
        }
    }
    String16(const uint16_t* s) {
        if (s) {
            size_t i = 0;
            while (s[i]) ++i;
            mString = (uint16_t*)malloc((i + 1) * sizeof(uint16_t));
            if (mString) memcpy(mString, s, (i + 1) * sizeof(uint16_t));
        } else mString = nullptr;
    }
    ~String16() { free(mString); }
    size_t size() const {
        if (!mString) return 0;
        size_t i = 0;
        while (mString[i]) ++i;
        return i;
    }
    size_t length() const { return size(); }
    const uint16_t* u16() const { return mString; }
    const char* string() const { return reinterpret_cast<const char*>(mString ? mString : (const uint16_t*)u""); }
};

struct native_handle {
    int version;
    int numFds;
    int numInts;
    int data[0];
};

inline native_handle* native_handle_create(int numFds, int numInts) {
    size_t size = sizeof(native_handle) + sizeof(int) * (numFds + numInts);
    native_handle* h = (native_handle*)malloc(size);
    if (h) {
        h->version = sizeof(native_handle);
        h->numFds = numFds;
        h->numInts = numInts;
    }
    return h;
}

inline void native_handle_delete(native_handle* h) {
    if (h) free(h);
}

struct ANativeWindowBuffer {};

struct android_ycbcr {
    void* y;
    void* cb;
    void* cr;
    uint32_t ystride;
    uint32_t cstride;
    uint32_t chroma_step;
};

struct Point {
    int32_t x, y;
    Point() : x(0), y(0) {}
    Point(int32_t x_, int32_t y_) : x(x_), y(y_) {}
};

template <typename T> class sp {
    T* m_ptr;
public:
    sp() : m_ptr(nullptr) {}
    sp(T* ptr) : m_ptr(ptr) {}
    sp(const sp& other) : m_ptr(other.m_ptr) {}
    ~sp() {}
    T* operator->() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    bool operator!() const { return m_ptr == nullptr; }
    operator bool() const { return m_ptr != nullptr; }
};

template <typename T> class wp {
    T* m_ptr;
public:
    wp() : m_ptr(nullptr) {}
    wp(T* ptr) : m_ptr(ptr) {}
    wp(const wp& other) : m_ptr(other.m_ptr) {}
    ~wp() {}
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
    virtual void* localBinder() { return nullptr; }
    virtual void* remoteBinder() { return nullptr; }
    virtual sp<IInterface> queryLocalInterface(const String16&) { return sp<IInterface>(nullptr); }
    static bool checkSubclass(const void*) { return true; }
    struct DeathRecipient {
        virtual ~DeathRecipient() {}
    };
};

struct VectorImpl {
    void* mArray;
    size_t mCount;
    size_t mCapacity;
    uint32_t mItemSize;
    uint32_t mFlags;
    VectorImpl(size_t itemSize, uint32_t flags) : mArray(nullptr), mCount(0), mCapacity(0), mItemSize(itemSize), mFlags(flags) {}
    VectorImpl(const VectorImpl& other) : mArray(nullptr), mCount(0), mCapacity(0), mItemSize(other.mItemSize), mFlags(other.mFlags) { }
    ~VectorImpl() { free(mArray); }
    VectorImpl& operator=(const VectorImpl& other) { clear(); return *this; }
    void* editArrayImpl() { return mArray; }
    void resize(size_t newCount) { mCount = newCount; }
    void appendVector(const VectorImpl& other) {}
    void add(const void* item) {}
    void clear() { mCount = 0; }
    void insertAt(const void* item, size_t index, size_t num) {}
};

template <typename T>
struct Vector : public VectorImpl {
    Vector() : VectorImpl(sizeof(T), 0) {}
    void do_destroy(void* array, size_t count) const {}
    void do_construct(void* array, size_t count) const {}
    void do_move_forward(void* dst, const void* src, size_t count) const { memmove(dst, src, count * sizeof(T)); }
    void do_move_backward(void* dst, const void* src, size_t count) const { memmove(dst, src, count * sizeof(T)); }
    void do_copy(void* dst, const void* src, size_t count) const { memcpy(dst, src, count * sizeof(T)); }
    void do_splat(void* dst, const void* item, size_t count) const {}
    const T* begin() const { return (const T*)mArray; }
    const T* end() const { return (const T*)((char*)mArray + mCount * sizeof(T)); }
    size_t size() const { return mCount; }
    T& operator[](size_t i) { return *((T*)((char*)mArray + i * sizeof(T))); }
    const T& operator[](size_t i) const { return *((T*)((char*)mArray + i * sizeof(T))); }
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
    bool operator==(const Rect& other) const {
        return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
    }
    Rect operator+(const Point& p) const { Rect r = *this; r.offsetBy(p.x, p.y); return r; }
    Rect operator-(const Point& p) const { Rect r = *this; r.offsetBy(-p.x, -p.y); return r; }
    void reduce(const Rect& other) {}
    Rect transform(uint32_t, int, int) const { return *this; }
    static const Rect EMPTY_RECT;
    static const Rect INVALID_RECT;
};

const Rect Rect::EMPTY_RECT = Rect(0,0,0,0);
const Rect Rect::INVALID_RECT = Rect(-1,-1,-1,-1);

template<typename T>
struct SimpleVector {
    T* data_;
    size_t size_;
    size_t capacity_;
    SimpleVector(): data_(nullptr), size_(0), capacity_(0) {}
    ~SimpleVector() { free(data_); }
    void push_back(const T& v) {
        if (size_ + 1 > capacity_) {
            size_t newcap = capacity_ ? capacity_ * 2 : 4;
            T* n = (T*)realloc(data_, newcap * sizeof(T));
            if (!n) return;
            data_ = n;
            capacity_ = newcap;
        }
        data_[size_++] = v;
    }
    void append(const SimpleVector& other) {
        for (size_t i = 0; i < other.size_; ++i) push_back(other.data_[i]);
    }
    size_t size() const { return size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ ? data_ + size_ : nullptr; }
    void clear() { size_ = 0; }
    void orSelf(const T& r) { push_back(r); }
    void orSelf(const SimpleVector& other) { append(other); }
    void orSelf(const SimpleVector& other, int dx, int dy) {
        for (size_t i = 0; i < other.size_; ++i) {
            T rr = other.data_[i];
            rr.offsetBy(dx, dy);
            push_back(rr);
        }
    }
    const T& operator[](size_t i) const { return data_[i]; }
    T& operator[](size_t i) { return data_[i]; }
};

struct Region {
    SimpleVector<Rect> mRects;
    Region() {}
    Region(const Rect& r) { mRects.push_back(r); }
    Region(const Region& other) { mRects.append(other.mRects); }
    ~Region() {}
    Region& operator=(const Region& other) { mRects.clear(); mRects.append(other.mRects); return *this; }
    void clear() { mRects.clear(); }
    void set(const Rect& r) { mRects.clear(); mRects.push_back(r); }
    void set(int w, int h) { set(Rect(0,0,w,h)); }
    void set(uint32_t w, uint32_t h) { set(static_cast<int>(w), static_cast<int>(h)); }
    void orSelf(const Rect& r) { mRects.orSelf(r); }
    void orSelf(const Region& other) { mRects.orSelf(other.mRects); }
    void orSelf(const Region& other, int dx, int dy) { mRects.orSelf(other.mRects, dx, dy); }
    void andSelf(const Rect& r) {}
    void andSelf(const Region& other) {}
    void andSelf(const Region& other, int dx, int dy) {}
    void xorSelf(const Rect& r) {}
    void xorSelf(const Region& other) {}
    void xorSelf(const Region& other, int dx, int dy) {}
    void subtractSelf(const Rect& r) {}
    void subtractSelf(const Region& other) {}
    void subtractSelf(const Region& other, int dx, int dy) {}
    void translateSelf(int dx, int dy) { for (size_t i = 0; i < mRects.size(); ++i) mRects[i].offsetBy(dx, dy); }
    void makeBoundsSelf() {}
    void addRectUnchecked(int l, int t, int r, int b) { mRects.push_back(Rect(l,t,r,b)); }
    static void boolean_operation(int op, Region& out, const Region& a, const Rect& b) { out = a; }
    static void boolean_operation(int op, Region& out, const Region& a, const Rect& b, int dx, int dy) { out = a; }
    static void boolean_operation(int op, Region& out, const Region& a, const Region& b) { out = a; }
    static void boolean_operation(int op, Region& out, const Region& a, const Region& b, int dx, int dy) { out = a; }
    static Region createTJunctionFreeRegion(const Region& r) { return r; }
    void operationSelf(const Rect& r, int op) {}
    void operationSelf(const Region& other, int op) {}
    void operationSelf(const Region& other, int op1, int op2, int op3) {}
    static void translate(Region& out, const Region& in, int dx, int dy) { out = in; out.translateSelf(dx, dy); }
    static void translate(Region& out, int dx, int dy) { out.translateSelf(dx, dy); }
    void unflatten(const void* data, size_t len) {}
    Region mergeExclusive(const Rect& r) const { Region res = *this; res.orSelf(r); return res; }
    Region mergeExclusive(const Region& other) const { Region res = *this; res.orSelf(other); return res; }
    Region mergeExclusive(const Region& other, int dx, int dy) const { Region res = *this; res.orSelf(other, dx, dy); return res; }
    size_t getFlattenedSize() const { return mRects.size() * sizeof(Rect); }
    bool isTriviallyEqual(const Region& other) const { if (mRects.size() != other.mRects.size()) return false; for (size_t i = 0; i < mRects.size(); ++i) if (!(mRects[i] == other.mRects[i])) return false; return true; }
    const Rect* begin() const { return mRects.begin(); }
    const Rect* end() const { return mRects.end(); }
    String8 dump(const char* what, uint32_t flags) const { return String8("dump"); }
    void dump(String8& out, const char* what, uint32_t flags) const { out.append("dump"); }
    bool contains(const Point& p) const { return false; }
    bool contains(int x, int y) const { return contains(Point(x,y)); }
    uint32_t* getArray(uint32_t* count) const { if (count) *count = static_cast<uint32_t>(mRects.size()); return nullptr; }
    Region merge(const Rect& r) const { Region res = *this; res.orSelf(r); return res; }
    Region merge(const Region& other) const { Region res = *this; res.orSelf(other); return res; }
    Region merge(const Region& other, int dx, int dy) const { Region res = *this; res.orSelf(other, dx, dy); return res; }
    int flatten(void* buffer, size_t len) const { return 0; }
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
        uint32_t len = static_cast<uint32_t>(str.length());
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
        writeInt32(static_cast<bool>(binder) ? 1 : 0);
    }
    void writeInterfaceToken(const String16& token) {
        uint32_t len = static_cast<uint32_t>(token.size());
        writeUint32(len);
        if (len) write(token.mString, len * sizeof(uint16_t));
    }
    void writeBool(bool val) { writeInt32(val ? 1 : 0); }
    void writeFloat(float val) { write(&val, sizeof(val)); }
    int32_t readInt32() const {
        int32_t val = 0;
        if (mDataPos + sizeof(val) <= mDataSize) memcpy(&val, mData + mDataPos, sizeof(val));
        const_cast<Parcel*>(this)->mDataPos += sizeof(val);
        return val;
    }
    int64_t readInt64() const {
        int64_t val = 0;
        if (mDataPos + sizeof(val) <= mDataSize) memcpy(&val, mData + mDataPos, sizeof(val));
        const_cast<Parcel*>(this)->mDataPos += sizeof(val);
        return val;
    }
    uint32_t readUint32() const { return static_cast<uint32_t>(readInt32()); }
    uint64_t readUint64() const { return static_cast<uint64_t>(readInt64()); }
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
        if (h) {
            size_t need = sizeof(int) * (numFds + numInts);
            if (mDataPos + need <= mDataSize) memcpy(h->data, mData + mDataPos, need);
        }
        const_cast<Parcel*>(this)->mDataPos += sizeof(int) * (numFds + numInts);
        return h;
    }
    sp<IBinder> readStrongBinder() const {
        readInt32();
        return sp<IBinder>(nullptr);
    }
    bool readBool() const { return readInt32() != 0; }
    void readFloat(float* val) const {
        if (mDataPos + sizeof(float) <= mDataSize) memcpy(val, mData + mDataPos, sizeof(float));
        const_cast<Parcel*>(this)->mDataPos += sizeof(float);
    }
    bool checkInterface(IBinder* binder) const { return true; }
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
    void unflatten(const void*&, size_t&, const int*&, size_t&) {}
    int64_t getSignalTime() const { return 0; }
};

struct GraphicBuffer : public RefBase {
    uint32_t width, height;
    int32_t format;
    uint32_t usage, stride;
    native_handle* handle;
    GraphicBuffer() : width(0), height(0), format(0), usage(0), stride(0), handle(nullptr) {}
    GraphicBuffer(uint32_t w, uint32_t h, int32_t f, uint32_t u) : width(w), height(h), format(f), usage(u), stride(0), handle(nullptr) {}
    GraphicBuffer(uint32_t w, uint32_t in_h, int32_t f, uint32_t u, uint32_t s, native_handle* h, bool keep) : width(w), height(in_h), format(f), usage(u), stride(s), handle(h) {}
    GraphicBuffer(ANativeWindowBuffer* buf, bool keep) {}
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
    void unflatten(const void*&, size_t&, const int*&, size_t&) {}
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

struct IGraphicBufferConsumer : public IInterface {
    static const String16 descriptor;
    virtual String16 getInterfaceDescriptor() const { return descriptor; }
    IGraphicBufferConsumer() {}
    virtual ~IGraphicBufferConsumer() {}
};

const String16 IGraphicBufferConsumer::descriptor("android.IGraphicBufferConsumer");

template <typename INTERFACE>
struct BnInterface : public INTERFACE, public IBinder {
    virtual String16 getInterfaceDescriptor() const { return INTERFACE::getInterfaceDescriptor(); }
};

template <typename INTERFACE>
struct BpRefBase {
    BpRefBase(const sp<IBinder>& o) {}
    virtual ~BpRefBase() {}
};

template <typename INTERFACE>
struct BpInterface : public INTERFACE, public BpRefBase<INTERFACE> {
    BpInterface(const sp<IBinder>& remote) : BpRefBase<INTERFACE>(remote) {}
};

struct IDumpTunnel : public IInterface {
    static const String16 descriptor;
    virtual String16 getInterfaceDescriptor() const { return descriptor; }
};

const String16 IDumpTunnel::descriptor("android.IDumpTunnel");

struct OccupancyTracker {
    struct Segment {};
};

struct Parcelable {
    virtual int writeToParcel(Parcel* p) const = 0;
    virtual int readFromParcel(const Parcel* p) = 0;
    virtual ~Parcelable() {}
};

struct HdrCapabilities : public Parcelable {
    int writeToParcel(Parcel* p) const override { return 0; }
    int readFromParcel(const Parcel* p) override { return 0; }
};

struct ComposerState {};

struct DisplayState {};

struct ComposerService {
    static ComposerService& get() { static ComposerService instance; return instance; }
};

struct Composer {
    static Composer& get() { static Composer instance; return instance; }
    sp<IBinder> createDisplay(const String8&) { return sp<IBinder>(nullptr); }
    void destroyDisplay(const sp<IBinder>&) {}
    void setDisplaySize(const sp<IBinder>&, uint32_t, uint32_t) {}
    sp<IBinder> getBuiltInDisplay(int) { return sp<IBinder>(nullptr); }
    void setDisplaySurface(const sp<IBinder>&, const sp<IGraphicBufferConsumer>&) {}
    void setBlurMaskSurface(const sp<IBinder>&, const sp<IBinder>&, const sp<IBinder>&) {}
    void setBlurMaskSampling(const sp<IBinder>&, const sp<IBinder>&, uint32_t) {}
    void deferTransactionUntil(const sp<IBinder>&, const sp<IBinder>&, const sp<IBinder>&, uint64_t) {}
    void setDisplayLayerStack(const sp<IBinder>&, uint32_t) {}
    void setDisplayProjection(const sp<IBinder>&, uint32_t, const Rect&, const Rect&) {}
    void setOverrideScalingMode(const sp<IBinder>&, int) {}
    void setTransparentRegionHint(const sp<IBinder>&, const Region&) {}
    void openGlobalTransactionImpl() {}
    void setBlurMaskAlphaThreshold(const sp<IBinder>&, const sp<IBinder>&, float) {}
    void closeGlobalTransactionImpl(bool) {}
    void setAnimationTransactionImpl() {}
    void setGeometryAppliesWithResize(const sp<IBinder>&, const sp<IBinder>&) {}
    void setBlur(const sp<IBinder>&, const sp<IBinder>&, float) {}
    void setCrop(const sp<IBinder>&, const sp<IBinder>&, const Rect&) {}
    void setSize(const sp<IBinder>&, const sp<IBinder>&, uint32_t, uint32_t) {}
    void setAlpha(const sp<IBinder>&, const sp<IBinder>&, float) {}
    void setColor(const sp<IBinder>&, const sp<IBinder>&, uint32_t) {}
    void setFlags(const sp<IBinder>&, const sp<IBinder>&, uint32_t, uint32_t) {}
    void setLayer(const sp<IBinder>&, const sp<IBinder>&, uint32_t) {}
    void setMatrix(const sp<IBinder>&, const sp<IBinder>&, float, float, float, float) {}
    void setPosition(const sp<IBinder>&, const sp<IBinder>&, float, float) {}
    ComposerState getLayerStateLocked(const sp<IBinder>&, const sp<IBinder>&) { return ComposerState{}; }
    DisplayState getDisplayStateLocked(const sp<IBinder>&) { return DisplayState{}; }
};

struct SurfaceComposerClient : public RefBase {
    sp<IBinder> connection() const { return sp<IBinder>(nullptr); }
    status_t initCheck() const { return 0; }
    virtual ~SurfaceComposerClient() {}
};

struct SyncFeatures {
    SyncFeatures() {}
    String8 toString() const { return String8(""); }
};

struct BufferQueueCore {};

struct BufferQueueDebug {
    virtual ~BufferQueueDebug() {}
};

struct BufferQueueMonitor {
    void dump(String8&, const char*) {}
    virtual ~BufferQueueMonitor() {}
};

struct GuiExtMonitor {};

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

} // namespace android

namespace shim {

struct GlobalResolvers {
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    bool resolved = false;
    void* libgui = nullptr;
    void* libui = nullptr;
    void resolve() {
        pthread_mutex_lock(&mutex);
        if (resolved) {
            pthread_mutex_unlock(&mutex);
            return;
        }
        resolved = true;
        pthread_mutex_unlock(&mutex);
        libgui = dlopen("/system/lib/libgui.so", RTLD_LAZY | RTLD_LOCAL);
        if (!libgui) libgui = dlopen("/system/lib64/libgui.so", RTLD_LAZY | RTLD_LOCAL);
        if (!libgui) libgui = dlopen("libgui.so", RTLD_LAZY | RTLD_LOCAL);
        libui = dlopen("/system/lib/libui.so", RTLD_LAZY | RTLD_LOCAL);
        if (!libui) libui = dlopen("/system/lib64/libui.so", RTLD_LAZY | RTLD_LOCAL);
        if (!libui) libui = dlopen("libui.so", RTLD_LAZY | RTLD_LOCAL);
    }
};

static GlobalResolvers g_resolvers;

} // namespace shim
