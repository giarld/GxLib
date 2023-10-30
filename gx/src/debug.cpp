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

#include "gx/debug.h"

#include <cstdarg>

#include <gx/gtime.h>
#include <gx/gthread.h>


#if GX_PLATFORM_ANDROID
#include <android/log.h>
#endif // GX_PLATFORM_ANDROID

GX_NS_BEGIN

#define DEBUG_CHAR_BUFFER_SIZE 16384

void defaultOutputWriterFunc(int level, const char *buffer)
{
#if GX_PLATFORM_ANDROID
    switch (level) {
        case 0:
            __android_log_print(ANDROID_LOG_INFO, "Gx", "%s [INFO] %s", GTime::currentSystemTime().toString("yyyy-MM-dd HH:mm:ss.zzz").c_str(), buffer);
            break;
        case 1:
            __android_log_print(ANDROID_LOG_DEBUG, "Gx", "%s [DEBUG] %s", GTime::currentSystemTime().toString("yyyy-MM-dd HH:mm:ss.zzz").c_str(), buffer);
            break;
        case 2:
            __android_log_print(ANDROID_LOG_WARN, "Gx", "%s [WARN] %s", GTime::currentSystemTime().toString("yyyy-MM-dd HH:mm:ss.zzz").c_str(), buffer);
            break;
        case 3:
            __android_log_print(ANDROID_LOG_ERROR, "Gx", "%s [ERROR] %s", GTime::currentSystemTime().toString("yyyy-MM-dd HH:mm:ss.zzz").c_str(), buffer);
            break;
        default:
            break;
    }
#else
    switch (level) {
        case 0:
            fprintf(stdout, "%s [INFO] %s\n", GTime::currentSystemTime().toString("yyyy-MM-dd HH:mm:ss.zzz").c_str(), buffer);
            break;
        case 1:
            fprintf(stdout, "%s [DEBUG] %s\n", GTime::currentSystemTime().toString("yyyy-MM-dd HH:mm:ss.zzz").c_str(), buffer);
            break;
        case 2:
            fprintf(stdout, "%s [WARN] %s\n", GTime::currentSystemTime().toString("yyyy-MM-dd HH:mm:ss.zzz").c_str(), buffer);
            break;
        case 3:
            fprintf(stderr, "%s [ERROR] %s\n", GTime::currentSystemTime().toString("yyyy-MM-dd HH:mm:ss.zzz").c_str(), buffer);
            break;
        default:
            break;
    }
#endif
}

static DebugOutputWriter sWriterFunc = defaultOutputWriterFunc;

void debugPrintf(int level, const char *format, ...)
{
    thread_local std::vector<char> buffer(DEBUG_CHAR_BUFFER_SIZE);

    va_list argList;

    va_start(argList, format);
    ::vsnprintf(buffer.data(), DEBUG_CHAR_BUFFER_SIZE, format, argList);
    va_end(argList);

    if (sWriterFunc) {
        sWriterFunc(level, buffer.data());
    }
}

void setDebugOutputWriter(DebugOutputWriter writer)
{
    if (writer) {
        sWriterFunc = writer;
    } else {
        sWriterFunc = defaultOutputWriterFunc;
    }
}

GX_NS_END
