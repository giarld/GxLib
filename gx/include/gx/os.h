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

#ifndef GX_OS_H
#define GX_OS_H

#include "gx/base.h"
#include "gx/gglobal.h"

#include <string>
#include <vector>


GX_NS_BEGIN

namespace os
{

GX_API void *dlOpen(const std::string &path);

GX_API void dlClose(void *handle);

GX_API void *dlSym(void *handle, const std::string &symbol);

GX_API std::string getEnv(const std::string &name);

GX_API std::vector<std::string> getEnvPATH();

/**
 * @brief Get operating system name
 * @return Windows|Linux|MacOS|IOS|UWP|Android|BSD|Web
 */
GX_API std::string name();

/**
 * @brief Get CPU type name
 * @return ARM|MIPS|PowerPC|RISC-V|x86
 */
GX_API std::string cpuName();

/**
 * @brief Get CPU architecture bit width name
 * @return 32-bit|64-bit
 */
GX_API std::string archName();

GX_API int execute(const std::string &cmd);

}

GX_NS_END

#endif //GX_OS_H
