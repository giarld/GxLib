/*
 * Copyright (c) 2021 Gxin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef GX_ALLOCATOR_H
#define GX_ALLOCATOR_H

#include "gx/base.h"
#include "gx/gglobal.h"

#include "memalign.h"
#include "gx/gmutex.h"
#include "debug.h"

#include <memory.h>
#include <atomic>
#include <type_traits>


GX_NS_BEGIN

namespace pointer
{

static inline size_t alignSize(size_t size, size_t alignment)
{
    return (size + (alignment - 1)) & ~(alignment - 1);
}

template<typename P, typename T>
static inline P *add(P *a, T b) noexcept
{
    return (P *) (uintptr_t(a) + uintptr_t(b));
}

template<typename P>
static inline P *align(P *p, size_t alignment) noexcept
{
    // Alignment must be a power of 2
    GX_ASSERT(alignment && !(alignment & alignment - 1));
    return (P *) ((uintptr_t(p) + alignment - 1) & ~(alignment - 1));
}

template<typename P>
static inline P *align(P *p, size_t alignment, size_t offset) noexcept
{
    P *const r = align(add(p, offset), alignment);
    GX_ASSERT(r >= add(p, offset));
    return r;
}

}


/**
 * @class LinearAllocator
 * @brief Cannot free individual blocks.
 * There is no limit on the size of each allocation, but it cannot be larger than the pre allocated space.
 * Need to manually reset the allocation pointer, suitable for memory allocation within one frame.
 */
class LinearAllocator
{
public:
    LinearAllocator() = default;

    /**
     * @brief Use the provided buffer.
     * @param begin
     * @param end
     */
    LinearAllocator(void *begin, void *end) noexcept
            : mBegin(begin), mSize(uintptr_t(end) - uintptr_t(begin))
    {
    }

    template<typename AREA>
    explicit LinearAllocator(const AREA &area)
            : LinearAllocator(area.begin(), area.end())
    {}

    /**
     * @brief Allocators can't be copied.
     * @param rhs
     */
    LinearAllocator(const LinearAllocator &rhs) = delete;

    LinearAllocator &operator=(const LinearAllocator &rhs) = delete;

    /**
     * @brief Allocators can be moved.
     * @param rhs
     */
    LinearAllocator(LinearAllocator &&rhs) noexcept
    {
        this->swap(rhs);
    }

    LinearAllocator &operator=(LinearAllocator &&rhs) noexcept
    {
        if (this != &rhs) {
            this->swap(rhs);
        }
        return *this;
    }

    ~LinearAllocator() noexcept = default;

public:
    /**
     * our allocator concept.
     * @param size
     * @param alignment
     * @param extra
     * @return
     */
    void *alloc(size_t size, size_t alignment = alignof(std::max_align_t), size_t extra = 0)
    {
        // branch-less allocation
        void *const p = pointer::align(current(), alignment, extra);
        void *const c = pointer::add(p, size);
        bool success = c <= end();
        set_current(success ? c : current());
        return success ? p : nullptr;
    }

    /**
     * @brief Get current header pointer.
     * Unique to this dispenser.
     *
     * @return
     */
    void *getCurrent() noexcept
    {
        return current();
    }

    /**
     * @brief Fallback memory pointer to a specific point
     *
     * @param p
     */
    void rewind(void *p) noexcept
    {
        GX_ASSERT(p >= mBegin && p < end());
        set_current(p);
    }

    /**
     * @brief Roll back memory pointer to the end
     */
    void reset() noexcept
    {
        rewind(mBegin);
    }

    size_t size() const noexcept
    {
        return mCur;
    }

    size_t capacity() const noexcept
    {
        return mSize;
    }

    void swap(LinearAllocator &rhs) noexcept
    {
        std::swap(mBegin, rhs.mBegin);
        std::swap(mSize, rhs.mSize);
        std::swap(mCur, rhs.mCur);
    }

    void *base() noexcept
    { return mBegin; }

    void free(void *, size_t) noexcept
    {}

private:
    void *end() noexcept
    { return pointer::add(mBegin, mSize); }

    void *current() noexcept
    { return pointer::add(mBegin, mCur); }

    void set_current(void *p) noexcept
    { mCur = uint32_t(uintptr_t(p) - uintptr_t(mBegin)); }

private:
    void *mBegin = nullptr;
    uint32_t mSize = 0;
    uint32_t mCur = 0;
};


/**
 * @class HeapAllocator
 * @brief Standard heap memory allocator.
 *
 * Using alignedAlloc to allocate memory
 * Reclaiming memory using alignedFree
 */
class HeapAllocator
{
public:
    HeapAllocator() noexcept = default;

    template<typename AREA>
    explicit HeapAllocator(const AREA &)
    {}

    HeapAllocator(const HeapAllocator &rhs) = delete;

    HeapAllocator &operator=(const HeapAllocator &rhs) = delete;

    HeapAllocator(HeapAllocator &&rhs) noexcept = default;

    HeapAllocator &operator=(HeapAllocator &&rhs) noexcept = default;

    ~HeapAllocator() noexcept = default;

public:
    void *alloc(size_t size, size_t alignment = alignof(std::max_align_t), size_t extra = 0)
    {
        // This allocator does not support 'extra'
        GX_ASSERT(extra == 0);
        return alignedAlloc(size, alignment);
    }

    void free(void *p) noexcept
    {
        alignedFree(p);
    }

    void free(void *p, size_t) noexcept
    {
        this->free(p);
    }

    void swap(HeapAllocator &rhs) noexcept
    {}

    size_t size() const noexcept
    {
        return 0;
    }

    size_t capacity() const noexcept
    {
        return 0;
    }
};

// ------------------------------------------------------------------------------------------------

class FreeList
{
public:
    FreeList() noexcept = delete;

    FreeList(void *begin, void *end, size_t elementSize, size_t alignment, size_t extra) noexcept
            : mElementSize(elementSize),
              mAlignment(alignment),
              mHead(init(begin, end, elementSize, alignment, extra)),
              mUserBegin(begin),
              mUserEnd(end)
    {
    }

    FreeList(size_t elementSize, size_t alignment) noexcept
            : mElementSize(elementSize),
              mAlignment(alignment)
    {
    }

    FreeList(const FreeList &rhs) = delete;

    ~FreeList()
    {
        clear();
    }

    FreeList &operator=(const FreeList &rhs) = delete;

    FreeList(FreeList &&rhs) noexcept = default;

    FreeList &operator=(FreeList &&rhs) noexcept = default;

public:
    void *pop() noexcept
    {
        if (mHead == nullptr) {
            Node *const head = createNode();
            GX_ASSERT(head);
            if (head == nullptr) {
                return nullptr;
            }
            head->next = nullptr;
            mHead = head;
        }
        Node *const head = mHead;
        mHead = (head ? head->next : nullptr);
        ++mAllocCount;
        return head;
    }

    void push(void *p) noexcept
    {
        GX_ASSERT(p);
        if (p == nullptr) {
            return;
        }
        Node *const head = static_cast<Node *>(p);
        head->next = mHead;
        mHead = head;
        --mAllocCount;
    }

    void *getFirst() noexcept
    {
        return mHead;
    }

    void clear() noexcept
    {
        Node *userNodeHead = nullptr;
        while (mHead) {
            Node *next = mHead->next;
            if (mHead >= mUserBegin && mHead < mUserEnd) {
                mHead->next = userNodeHead;
                userNodeHead = mHead;
            } else {
                alignedFree(mHead);
            }
            mHead = next;
        }
        mHead = userNodeHead;
    }

    size_t size() const noexcept
    {
        return mAllocCount * mElementSize;
    }

    size_t capacity() const noexcept
    {
        size_t count = 0;
        for (Node *c = mHead; c != nullptr; c = c->next) {
            ++count;
        }
        return count * mElementSize + size();
    }

private:
    struct Node
    {
        Node *next;
    };

    static Node *init(void *begin, void *end, size_t elementSize, size_t alignment, size_t extra) noexcept
    {
        void *const p = pointer::align(begin, alignment, extra);
        void *const n = pointer::align(pointer::add(p, elementSize), alignment, extra);
        GX_ASSERT(p >= begin && p < end);
        GX_ASSERT(n >= begin && n < end && n > p);

        const size_t d = uintptr_t(n) - uintptr_t(p);
        const size_t num = (uintptr_t(end) - uintptr_t(p)) / d;

        Node *head = static_cast<Node *>(p);

        Node *cur = head;
        for (size_t i = 1; i < num; ++i) {
            Node *next = pointer::add(cur, d);
            cur->next = next;
            cur = next;
        }
        GX_ASSERT(cur < end);
        GX_ASSERT(pointer::add(cur, d) <= end);
        cur->next = nullptr;
        return head;
    }

    Node *createNode() const noexcept
    {
        return static_cast<Node *>(alignedAlloc(mElementSize, mAlignment));
    }

private:
    size_t mElementSize = 0;
    size_t mAlignment = 0;
    Node *mHead = nullptr;
    void *mUserBegin = nullptr;
    void *mUserEnd = nullptr;
    size_t mAllocCount = 0;
};

// ------------------------------------------------------------------------------------------------

/**
 * @class PoolAllocator
 *
 * For memory pool allocation of fixed large and small blocks,
 * Only memory within a fixed size can be allocated at a time,
 * A certain size of continuous memory can be pre allocated as the initial memory block of the pool,
 * The default FreeList will allocate more memory after the original memory block is exhausted,
 * These memory that has not been allocated can be reclaimed using reset, and will also be reclaimed during PoolAllocator deconstruction,
 * Before disassembling PoolAllocator, it is important to free all allocated memory blocks, otherwise it will cause memory leakage
 *
 * @tparam ELEMENT_SIZE Element size (byte) must be greater than or equal to sizeof (void *)
 * @tparam ALIGNMENT    Alignment during element memory allocation
 * @tparam OFFSET       Offset for element memory alignment
 */
template<size_t ELEMENT_SIZE,
        size_t ALIGNMENT = alignof(std::max_align_t),
        size_t OFFSET = 0>
class PoolAllocator
{
private:
    /// At least one Node pointer size must be accommodated
    static_assert(ELEMENT_SIZE >= sizeof(void *), "ELEMENT_SIZE must accommodate at least a pointer");

public:
    PoolAllocator() noexcept
            : mFreeList(ELEMENT_SIZE, ALIGNMENT)
    {
    }

    PoolAllocator(void *begin, void *end) noexcept
            : mFreeList(begin, end, ELEMENT_SIZE, ALIGNMENT, OFFSET)
    {
    }

    template<typename AREA>
    explicit PoolAllocator(const AREA &area) noexcept
            : PoolAllocator(area.begin(), area.end())
    {
    }

    PoolAllocator(const PoolAllocator &rhs) = delete;

    PoolAllocator &operator=(const PoolAllocator &rhs) = delete;

    PoolAllocator(PoolAllocator &&rhs) noexcept = default;

    PoolAllocator &operator=(PoolAllocator &&rhs) noexcept = default;

    ~PoolAllocator() noexcept = default;

public:
    void *alloc(size_t size = ELEMENT_SIZE,
                size_t alignment = ALIGNMENT, size_t offset = OFFSET) noexcept
    {
        GX_ASSERT(size <= ELEMENT_SIZE);
        GX_ASSERT(alignment <= ALIGNMENT);
        GX_ASSERT(offset == OFFSET);
        return mFreeList.pop();
    }

    void free(void *p, size_t = ELEMENT_SIZE) noexcept
    {
        mFreeList.push(p);
    }

    size_t size() const noexcept
    {
        return mFreeList.size();
    }

    size_t capacity() const noexcept
    {
        return mFreeList.capacity();
    }

    void *getCurrent() noexcept
    {
        return mFreeList.getFirst();
    }

    void reset() noexcept
    {
        mFreeList.clear();
    }

private:
    FreeList mFreeList;
};

template<typename T, size_t ALIGNMENT = alignof(T), size_t OFFSET = 0>
using ObjectPoolAllocator = PoolAllocator<sizeof(T), ALIGNMENT, OFFSET>;

// ------------------------------------------------------------------------------------------------
// Areas
// ------------------------------------------------------------------------------------------------

class HeapArea
{
public:
    HeapArea() noexcept = default;

    explicit HeapArea(size_t size)
    {
        if (size) {
            mBegin = malloc(size);
            mEnd = pointer::add(mBegin, size);
        }
    }

    ~HeapArea() noexcept
    {
        free(mBegin);
    }

    HeapArea(const HeapArea &rhs) = delete;

    HeapArea &operator=(const HeapArea &rhs) = delete;

    HeapArea(HeapArea &&rhs) noexcept = delete;

    HeapArea &operator=(HeapArea &&rhs) noexcept = delete;

public:
    void *data() const noexcept
    { return mBegin; }

    void *begin() const noexcept
    { return mBegin; }

    void *end() const noexcept
    { return mEnd; }

    size_t size() const noexcept
    { return uintptr_t(mEnd) - uintptr_t(mBegin); }

    friend void swap(HeapArea &lhs, HeapArea &rhs) noexcept
    {
        using std::swap;
        swap(lhs.mBegin, rhs.mBegin);
        swap(lhs.mEnd, rhs.mEnd);
    }

private:
    void *mBegin = nullptr;
    void *mEnd = nullptr;
};

class StaticArea
{
public:
    StaticArea() noexcept = default;

    StaticArea(void *b, void *e) noexcept
            : mBegin(b), mEnd(e)
    {
    }

    ~StaticArea() noexcept = default;

    StaticArea(const StaticArea &rhs) = default;

    StaticArea &operator=(const StaticArea &rhs) = default;

    StaticArea(StaticArea &&rhs) noexcept = default;

    StaticArea &operator=(StaticArea &&rhs) noexcept = default;

public:
    void *data() const noexcept
    { return mBegin; }

    void *begin() const noexcept
    { return mBegin; }

    void *end() const noexcept
    { return mEnd; }

    size_t size() const noexcept
    { return uintptr_t(mEnd) - uintptr_t(mBegin); }

    friend void swap(StaticArea &lhs, StaticArea &rhs) noexcept
    {
        using std::swap;
        swap(lhs.mBegin, rhs.mBegin);
        swap(lhs.mEnd, rhs.mEnd);
    }

private:
    void *mBegin = nullptr;
    void *mEnd = nullptr;
};

class NullArea
{
public:
    void *data() const noexcept
    { return nullptr; }

    size_t size() const noexcept
    { return 0; }
};

// ------------------------------------------------------------------------------------------------
// Policies
// ------------------------------------------------------------------------------------------------

namespace LockingPolicy
{

using NoLock = gx::GNoLock;

using Mutex = gx::GMutex;

using SpinLock = gx::GSpinLock;

} // namespace LockingPolicy


// ------------------------------------------------------------------------------------------------
// Ponds
// ------------------------------------------------------------------------------------------------

using UniquePtrDeleter = std::function<void(void *)>;

template<typename T>
using UniquePtr = std::unique_ptr<T, UniquePtrDeleter>;

template<typename AllocatorPolicy, typename LockingPolicy, typename AreaPolicy = HeapArea>
class Pond
{
public:
    explicit Pond(const char *name = "")
            : mName(name)
    {
    }

    /**
     * Construct a Pond with a name and allocator parameters.
     * @tparam ARGS
     * @param name
     * @param size
     * @param args
     */
    template<typename ... ARGS>
    Pond(const char *name, size_t size, ARGS &&... args)
            : mArea(size),
              mAllocator(mArea, std::forward<ARGS>(args) ...),
              mName(name)
    {
    }

    template<typename ... ARGS>
    Pond(const char *name, AreaPolicy &&area, ARGS &&... args)
            : mArea(std::forward<AreaPolicy>(area)),
              mAllocator(mArea, std::forward<ARGS>(args) ...),
              mName(name)
    {
    }

    Pond(const Pond &rhs) noexcept = delete;

    Pond &operator=(const Pond &rhs) noexcept = delete;

    Pond &operator=(Pond &&rhs) noexcept
    {
        if (this != &rhs) {
            swap(*this, rhs);
        }
        return *this;
    }

public:
    /**
     * Allocate memory based on specified size and alignment
     * Acceptable size/alignment may depend on the provided allocator.
     * @param size
     * @param alignment
     * @param extra
     * @return
     */
    void *alloc(size_t size, size_t alignment = alignof(std::max_align_t), size_t extra = 0) noexcept
    {
        GLockerGuard guard(mLock);
        void *p = mAllocator.alloc(size, alignment, extra);
        return p;
    }

    /**
     * Assign an array of destructible objects
     * For safety reasons, if the object type is not destructible,
     * we disable the object based alloc method because free() does not call the destructor.
     * @tparam T
     * @param count
     * @param alignment
     * @param extra
     * @return
     */
    template<typename T,
            typename = typename std::enable_if<std::is_trivially_destructible<T>::value>::type>
    T *alloc(size_t count, size_t alignment = alignof(T), size_t extra = 0) noexcept
    {
        return (T *) alloc(count * sizeof(T), alignment, extra);
    }

    /**
     * @brief Release memory.
     * @param p
     */
    void free(void *p) noexcept
    {
        if (p) {
            GLockerGuard guard(mLock);
            mAllocator.free(p);
        }
    }

    /**
     * @brief Release memory of specified size.
     * @param p
     * @param size
     */
    void free(void *p, size_t size) noexcept
    {
        if (p) {
            GLockerGuard guard(mLock);
            mAllocator.free(p, size);
        }
    }

    /**
     * @brief
     * Reset distributor.
     * 1 Some allocators do not have a free () call, but instead use a reset () or rewind () instead;
     * 2 PoolAllocator can reclaim allocated but unused memory by executing reset().
     */
    void reset() noexcept
    {
        GLockerGuard guard(mLock);
        mAllocator.reset();
    }

    void *getCurrent() noexcept
    { return mAllocator.getCurrent(); }

    void rewind(void *addr) noexcept
    {
        GLockerGuard guard(mLock);
        mAllocator.rewind(addr);
    }

    size_t size() const noexcept
    {
        GLockerGuard guard(mLock);
        return mAllocator.size();
    }

    size_t capacity() const noexcept
    {
        GLockerGuard guard(mLock);
        return mAllocator.capacity();
    }

    /**
     * @class Assign and construct an object.
     * @tparam T
     * @tparam ALIGN
     * @tparam ARGS
     * @param args
     * @return
     */
    template<typename T, size_t ALIGN = alignof(T), typename... ARGS>
    T *make(ARGS &&... args) noexcept
    {
        void *const p = this->alloc(sizeof(T), ALIGN);
        return p ? new(p) T(std::forward<ARGS>(args)...) : nullptr;
    }

    /**
     * @brief Deconstruct and release an object.
     * @tparam T
     * @param p
     */
    template<typename T>
    void destroy(T *p) noexcept
    {
        if (p) {
            p->~T();
            this->free((void *) p, sizeof(T));
        }
    }

    /**
     * @brief Allocate memory for the specified type and create an object and return shared_ Ptr,
     * the object will automatically destruct and reclaim memory after the reference count is reset to zero.
     * @tparam T
     * @tparam ALIGN
     * @tparam ARGS
     * @param args
     * @return
     */
    template<typename T, size_t ALIGN = alignof(T), typename... ARGS>
    std::shared_ptr<T> makeShared(ARGS &&... args) noexcept
    {
        void *const p = this->alloc(sizeof(T), ALIGN);
        if (p) {
            return std::shared_ptr<T>(new(p) T(std::forward<ARGS>(args)...), [this](void *ptr) {
                destroy(static_cast<T *>(ptr));
            });
        }
        return nullptr;
    }

    /**
     * @brief Allocate memory for the specified type and create an object and return unique_ Ptr,
     * the object will automatically destruct and reclaim memory after being abandoned by the owner.
     * @tparam T
     * @tparam ALIGN
     * @tparam ARGS
     * @param args
     * @return
     */
    template<typename T, size_t ALIGN = alignof(T), typename... ARGS>
    UniquePtr<T> makeUnique(ARGS &&... args) noexcept
    {
        void *const p = this->alloc(sizeof(T), ALIGN);
        if (p) {
            UniquePtrDeleter deleter = [this](void *ptr) {
                destroy(static_cast<T *>(ptr));
            };
            return std::unique_ptr<T, UniquePtrDeleter>(new(p) T(std::forward<ARGS>(args)...), deleter);
        }
        return nullptr;
    }

    const char *getName() const noexcept
    { return mName; }

    AllocatorPolicy &getAllocator() noexcept
    { return mAllocator; }

    const AllocatorPolicy &getAllocator() const noexcept
    { return mAllocator; }

    AreaPolicy &getArea() noexcept
    { return mArea; }

    const AreaPolicy &getArea() const noexcept
    { return mArea; }

    friend void swap(Pond &lhs, Pond &rhs) noexcept
    {
        using std::swap;
        swap(lhs.mArea, rhs.mArea);
        swap(lhs.mAllocator, rhs.mAllocator);
        swap(lhs.mLock, rhs.mLock);
        swap(lhs.mName, rhs.mName);
    }

private:
    AreaPolicy mArea;
    AllocatorPolicy mAllocator;
    mutable LockingPolicy mLock;
    const char *mName = nullptr;
};

// ------------------------------------------------------------------------------------------------

using HeapPond = Pond<HeapAllocator, LockingPolicy::NoLock>;


/**
 * Splitter packaging that can be used with STL container translators
 *
 * @tparam TYPE
 * @tparam POND
 */
template<typename TYPE, typename POND>
class STLAllocator
{
public:
    using value_type = TYPE;
    using pointer = TYPE *;
    using const_pointer = const TYPE *;
    using reference = TYPE &;
    using const_reference = const TYPE &;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::true_type;

    template<typename OTHER>
    struct rebind
    {
        using other = STLAllocator<OTHER, POND>;
    };

public:
    STLAllocator(POND &pond)
            : mPond(pond)
    {}

    template<typename U>
    explicit STLAllocator(const STLAllocator<U, POND> &rhs)
            : mPond(rhs.mPond)
    {}

public:
    TYPE *allocate(std::size_t n)
    {
        return static_cast<TYPE *>(mPond.alloc(n * sizeof(TYPE), alignof(TYPE)));
    }

    void deallocate(TYPE *p, std::size_t n)
    {
        mPond.free(p, n * sizeof(TYPE));
    }

    template<typename U, typename A>
    bool operator==(const STLAllocator<U, A> &rhs) const noexcept
    {
        return std::addressof(mPond) == std::addressof(rhs.mPond);
    }

    template<typename U, typename A>
    bool operator!=(const STLAllocator<U, A> &rhs) const noexcept
    {
        return !operator==(rhs);
    }

private:
    template<typename U, typename A>
    friend
    class STLAllocator;

    POND &mPond;
};

GX_NS_END

#endif //GX_ALLOCATOR_H
