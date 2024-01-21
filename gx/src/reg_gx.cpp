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

#include "gx/reg_gx.h"

#include "plugin/ref_gx.h"

#include "gx/gany.h"

#include "gx/debug.h"


using namespace gx;

static GAny sAnyDebugOutputWriter;

void anyDebugOutputWriter(int level, const char *buffer)
{
    if (sAnyDebugOutputWriter.isFunction()) {
        sAnyDebugOutputWriter(level, std::string(buffer));
    }
}

void logToAny()
{
    auto GLog = GAnyClass::Class("", "GLog", "gx log system");
    GAny::Export(GLog);
    GLog->staticFunc("Log", [](const std::string &msg) {
                    Log("%s", msg.c_str());
                })
                .staticFunc("LogD", [](const std::string &msg) {
                    LogD("%s", msg.c_str());
                })
                .staticFunc("LogW", [](const std::string &msg) {
                    LogW("%s", msg.c_str());
                })
                .staticFunc("LogE", [](const std::string &msg) {
                    LogE("%s", msg.c_str());
                })
                .staticFunc("setDebugOutputWriter", [](const GAny &writerFunc) {
                    if (writerFunc.isFunction()) {
                        sAnyDebugOutputWriter = writerFunc;
                        setDebugOutputWriter(anyDebugOutputWriter);
                    } else {
                        sAnyDebugOutputWriter = GAny();
                        setDebugOutputWriter(nullptr);
                    }
                }, "arg0: writerFunc - function(int level, string msg)");
}

REGISTER_GANY_MODULE(Gx)
{
    refGObject();
    refGFile();
    refGGlobalMemoryPool();
    refGByteArray();
    refGTime();
    refGThread();
    refGUuid();
    refGVersion();
    refTaskSystem();
    refGCrypto();
    refGHashSum();
    refGTimer();
    refOs();

    logToAny();
}
