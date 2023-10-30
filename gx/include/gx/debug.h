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

#ifndef GX_DEBUG_H
#define GX_DEBUG_H

#include "gx/base.h"
#include "gx/gglobal.h"

#include <cstdlib>
#include <cstdio>


#if GX_DEBUG
#   define _Log(level, fmt, ...)    \
    do {                    \
        gx::debugPrintf(level, __FILE__ "(" GX_STRINGIZE(__LINE__) ") : " fmt, ##__VA_ARGS__); \
    } while (false)
#else
#   define _Log(level, fmt, ...)   \
    do {                \
        gx::debugPrintf(level, " " fmt, ##__VA_ARGS__); \
    } while (false)
#endif

#define Log(fmt, ...)   _Log(0, fmt, ##__VA_ARGS__)
#define LogW(fmt, ...)  _Log(2, fmt, ##__VA_ARGS__)
#define LogE(fmt, ...)  _Log(3, fmt, ##__VA_ARGS__)

#if GX_DEBUG
#define LogD(fmt, ...)  _Log(1, fmt, ##__VA_ARGS__)
#else
#define LogD(fmt, ...)  ((void)0)
#endif

#define LogS(gstr)  Log("%s", gstr.c_str())

#if GX_DEBUG
#   define GX_ASSERT_S(Exp, fmt, ...) (void)(!!(Exp) || (gx::debugPrintf(3, "Assertion failed: " __FILE__ "(" GX_STRINGIZE(__LINE__) "), " #Exp ", " fmt, ##__VA_ARGS__), abort(), 0))
#   define GX_ASSERT(Exp) (void)(!!(Exp) || (gx::debugPrintf(3, "Assertion failed: " __FILE__ "(" GX_STRINGIZE(__LINE__) "), " #Exp), abort(), 0))
#else
#   define GX_ASSERT_S(Exp, fmt, ...) ((void)0)
#   define GX_ASSERT(Exp) ((void)0)
#endif //_DEBUG

GX_NS_BEGIN

typedef void(*DebugOutputWriter)(int level, const char *buffer);

/**
 * @brief print debug message
 * @param level     0: info, 1: debug, 2: warning, 3: error
 * @param format
 * @param ...
 */
GX_API void debugPrintf(int level, const char *format, ...);

GX_API void setDebugOutputWriter(DebugOutputWriter writer);

GX_NS_END

#endif //GX_DEBUG_H
