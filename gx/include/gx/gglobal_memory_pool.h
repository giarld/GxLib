/*
 * Copyright (c) 2023 Gxin
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

#ifndef GX_GLOBAL_MEMORY_POOL_H
#define GX_GLOBAL_MEMORY_POOL_H

#include "allocator.h"


GX_NS_BEGIN

/**
 * @class GGlobalMemoryPool
 * @brief Global memory pool, a memory pool provided by GByteArray.
 */
class GX_API GGlobalMemoryPool
{
    constexpr static uint32_t ELEMENT_S_SIZE = 8 * 1024;
    constexpr static uint32_t ELEMENT_M_SIZE = 64 * 1024;

public:
    static void *alloc(uint32_t &size);

    static void free(void *ptr, uint32_t size);

    static void gc();

    static uint64_t allocatedSize();

    static uint64_t poolCapacity();

    static uint64_t poolSize();

public:
    GGlobalMemoryPool(const GGlobalMemoryPool &) = delete;

    GGlobalMemoryPool(GGlobalMemoryPool &&) noexcept = delete;

    GGlobalMemoryPool &operator=(const GGlobalMemoryPool &) = delete;

    GGlobalMemoryPool &operator=(GGlobalMemoryPool &&) noexcept = delete;

private:
    explicit GGlobalMemoryPool();

    static GGlobalMemoryPool *getInstance();

    void *_alloc(uint32_t &size);

    void _free(void *ptr, uint32_t size);

    void _gc();

    uint64_t _allocatedSize();

    uint64_t _poolCapacity();

    uint64_t _poolSize();

private:
    using HeadPond = Pond<HeapAllocator, LockingPolicy::NoLock>;
    using PoolPondA = Pond<PoolAllocator<ELEMENT_S_SIZE>, LockingPolicy::Mutex>;
    using PoolPondB = Pond<PoolAllocator<ELEMENT_M_SIZE>, LockingPolicy::Mutex>;

    HeadPond mHeapAlloc;
    PoolPondA mPoolAllocS;
    PoolPondB mPoolAllocM;

    std::atomic<uint64_t> mAllocatedSize{0};
};

GX_NS_END

#endif //GX_GLOBAL_MEMORY_POOL_H
