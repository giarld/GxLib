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

#include "ref_gx.h"

#include <gx/os.h>


GX_NS_BEGIN

void refOs()
{
    auto clazz = GAnyClass::Class("Gx", "Os", "");
    clazz->staticFunc("getEnv", &os::getEnv)
         .staticFunc("getEnvPATH", &os::getEnvPATH)
         .staticFunc("name", &os::name,
                    "Get operating system name. "
                    "return: Windows|Linux|MacOS|IOS|UWP|Android|BSD|Web.")
         .staticFunc("cpuName", &os::cpuName,
                    "Get CPU type name. "
                    "return: ARM|MIPS|PowerPC|RISC-V|x86.")
         .staticFunc("archName", &os::archName,
                    "Get CPU architecture bit width name. "
                    "return: 32-bit|64-bit.")
         .staticFunc("execute", &os::execute);
    GAnyClass::registerToEnv(clazz);
}

GX_NS_END