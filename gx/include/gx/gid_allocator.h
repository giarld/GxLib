/*
 * Copyright (c) 2020 Gxin
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

#ifndef GX_GIDALLOCATOR_H
#define GX_GIDALLOCATOR_H

#include "gx/base.h"
#include "gx/gglobal.h"

#include "gx/gmutex.h"

#include <limits>


GX_NS_BEGIN

/**
 * ID allocator
 * ID fast allocation algorithm
 */
template<typename ID_TYPE, ID_TYPE MaxNum, typename MUTEX = GNoLock>
class GIDAllocator final
{
public:
    explicit GIDAllocator()
    {
        reset();
    }

public:
    inline void reset()
    {
        std::lock_guard locker(mMutex);

        mNum = 0;
        memset(mSparse, 0, sizeof(ID_TYPE) * MaxNum);
        for (ID_TYPE i = 0; i < MaxNum; i++) {
            mDense[i] = i;
        }
    }

    ID_TYPE alloc()
    {
        std::lock_guard locker(mMutex);

        if (mNum < MaxNum)
        {
            ID_TYPE index = mNum;
            ++mNum;
            ID_TYPE handle = mDense[index];
            mSparse[handle] = index;
            return handle;
        }
        return std::numeric_limits<ID_TYPE>::max();
    }

    void free(ID_TYPE id)
    {
        if (!isValid(id)) {
            return;
        }

        std::lock_guard locker(mMutex);

        ID_TYPE index = mSparse[id];
        --mNum;
        uint16_t temp = mDense[mNum];
        mDense[mNum] = id;
        mSparse[temp] = index;
        mDense[index] = temp;
    }

    bool isValid(ID_TYPE id)
    {
        if (id >= MaxNum) {
            return false;
        }
        uint16_t index = mSparse[id];
        return index < mNum && mDense[index] == id;
    }

private:
    ID_TYPE mNum = 0;

    ID_TYPE mDense[MaxNum];
    ID_TYPE mSparse[MaxNum];

    MUTEX mMutex;
};

GX_NS_END
#endif //GX_GIDALLOCATOR_H
