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

#ifndef GX_MEMALIGN_H
#define GX_MEMALIGN_H

#include "gx/base.h"
#include "gx/platform.h"

#include "debug.h"

#include <type_traits>

#include <cstddef>
#include <cstdlib>

#if GX_PLATFORM_WINDOWS

#include <malloc.h>

#endif


GX_NS_BEGIN

inline void *alignedAlloc(size_t size, size_t align) noexcept
{
    align = (align < sizeof(void*)) ? sizeof(void*) : align;
    GX_ASSERT(align && !(align & align - 1));
    GX_ASSERT((align % sizeof(void*)) == 0);

    void *p = nullptr;

#if GX_PLATFORM_WINDOWS
    p = ::_aligned_malloc(size, align);
#else
    ::posix_memalign(&p, align, size);
#endif
    return p;
}

inline void alignedFree(void *p) noexcept
{
#if GX_PLATFORM_WINDOWS
    ::_aligned_free(p);
#else
    ::free(p);
#endif
}

/*
 * This allocator can be used with std::vector to ensure that all projects are aligned with their alias()
 * E.g
 *
 *     template<typename T>
 *     using aligned_vector = std::vector<T, gx::STLAlignedAllocator<T>>;
 *
 *     aligned_vector<Foo> foos;
 *
 */
template<typename TYPE>
class STLAlignedAllocator
{
    static_assert(!(alignof(TYPE) & (alignof(TYPE) - 1)), "alignof(T) must be a power of two");

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

    template<typename T>
    struct rebind
    {
        using other = STLAlignedAllocator<T>;
    };

    inline STLAlignedAllocator() noexcept = default;

    template<typename T>
    inline explicit STLAlignedAllocator(const STLAlignedAllocator<T> &) noexcept
    {}

    inline ~STLAlignedAllocator() noexcept = default;

    inline pointer allocate(size_type n) noexcept
    {
        return (pointer) alignedAlloc(n * sizeof(value_type), alignof(TYPE));
    }

    inline void deallocate(pointer p, size_type)
    {
        alignedFree(p);
    }

    template<typename T>
    bool operator==(const STLAlignedAllocator<T> &rhs) const noexcept
    {
        return true;
    }

    template<typename T>
    bool operator!=(const STLAlignedAllocator<T> &rhs) const noexcept
    {
        return false;
    }
};

GX_NS_END

#endif //GX_MEMALIGN_H
