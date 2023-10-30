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

#include "gx/gglobal_memory_pool.h"

#include <memory>


#define POOL_S_PRE_ALLOC_SIZE (ELEMENT_S_SIZE * 32)
#define POOL_M_PRE_ALLOC_SIZE (ELEMENT_M_SIZE * 16)

GX_NS_BEGIN

GGlobalMemoryPool::GGlobalMemoryPool()
        : mHeapAlloc("GlobalHeapAlloc"),
          mPoolAllocS("GlobalPoolAllocS", POOL_S_PRE_ALLOC_SIZE),
          mPoolAllocM("GlobalPoolAllocM", POOL_M_PRE_ALLOC_SIZE)
{
}

void *GGlobalMemoryPool::alloc(uint32_t &size)
{
    return getInstance()->_alloc(size);
}

void GGlobalMemoryPool::free(void *ptr, uint32_t size)
{
    getInstance()->_free(ptr, size);
}

void GGlobalMemoryPool::gc()
{
    getInstance()->_gc();
}

uint64_t GGlobalMemoryPool::allocatedSize()
{
    return getInstance()->_allocatedSize();
}

uint64_t GGlobalMemoryPool::poolCapacity()
{
    return getInstance()->_poolCapacity();
}

uint64_t GGlobalMemoryPool::poolSize()
{
    return getInstance()->_poolSize();
}

GGlobalMemoryPool *GGlobalMemoryPool::getInstance()
{
    static auto *instance = GX_NEW(GGlobalMemoryPool);
    return instance;
}

void *GGlobalMemoryPool::_alloc(uint32_t &size)
{
    void *buffer;
    if (size <= ELEMENT_S_SIZE) { // <= 8k
        size = ELEMENT_S_SIZE;
        buffer = mPoolAllocS.alloc(size, alignof(uint8_t));
    } else if (size <= ELEMENT_M_SIZE) { // > 8k and <= 64k
        size = ELEMENT_M_SIZE;
        buffer = mPoolAllocM.alloc(size, alignof(uint8_t));
    } else { // > 64k
        buffer = mHeapAlloc.alloc(size, alignof(uint8_t));
    }
    if (buffer) {
        memset(buffer, 0, size);
    }
    mAllocatedSize += (uint64_t) size;
    return buffer;
}

void GGlobalMemoryPool::_free(void *ptr, uint32_t size)
{
    if (!ptr || size == 0) {
        return;
    }
    if (size <= ELEMENT_S_SIZE) { // <= 8k
        mPoolAllocS.free(ptr);
    } else if (size <= ELEMENT_M_SIZE) { // > 8k and <= 64k
        mPoolAllocM.free(ptr);
    } else { // > 64k
        mHeapAlloc.free(ptr);
    }
    mAllocatedSize -= (uint64_t) size;
}

void GGlobalMemoryPool::_gc()
{
    mPoolAllocS.reset();
    mPoolAllocM.reset();
}

uint64_t GGlobalMemoryPool::_allocatedSize()
{
    return mAllocatedSize.load();
}

uint64_t GGlobalMemoryPool::_poolCapacity()
{
    return mPoolAllocS.capacity() + mPoolAllocM.capacity();
}

uint64_t GGlobalMemoryPool::_poolSize()
{
    return mPoolAllocS.size() + mPoolAllocM.size();
}

GX_NS_END