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

#ifndef GX_ALGORITHM_MD5_H
#define GX_ALGORITHM_MD5_H

#include "gx/base.h"
#include <gx/gglobal.h>

GX_NS_BEGIN

#define MD5_BLOCK_SIZE 16

struct Md5Context
{
    unsigned int count[2];
    unsigned int state[4];
    unsigned char buffer[64];
};

void fMD5Init(Md5Context *context);

void fMD5Update(Md5Context *context, const uint8_t *input, uint32_t inputLen);

void fMD5Final(Md5Context *context, uint8_t digest[MD5_BLOCK_SIZE]);

GX_NS_END

#endif //GX_ALGORITHM_MD5_H
