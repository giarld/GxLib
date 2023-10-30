/*
 * Copyright (c) 2022 Gxin
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

#ifndef GX_HASH_SUM_H
#define GX_HASH_SUM_H

#include "gbytearray.h"

#include <memory>


GX_NS_BEGIN

class GHashJob
{
public:
    virtual ~GHashJob() {}

    virtual void update(const GByteArray &data) = 0;

    virtual void update(const uint8_t *data, uint32_t size) = 0;

    virtual GByteArray final() = 0;
};

/**
 * @class GHashSum
 * @brief Provide generation functions for Md5, Sha1, and Sha256
 */
class GX_API GHashSum
{
public:
    enum HashType
    {
        Md5 = 1,
        Sha1 = 2,
        Sha256 = 3,
    };

public:
    static std::unique_ptr<GHashJob> hashSum(GHashSum::HashType hashType);
};

GX_NS_END

#endif //GX_HASH_SUM_H
