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

#include "gx/os.h"

#include <gx/platform.h>

#include <gx/gstring.h>
#include <gx/debug.h>


#if GX_PLATFORM_WINDOWS

#include <windows.h>

#else

#include <dlfcn.h>

#endif

GX_NS_BEGIN
namespace os
{

void *dlOpen(const std::string &path)
{
#if GX_PLATFORM_WINDOWS
    GWString wStr = GString(path).toUtf16();
    return (void *) ::LoadLibraryW(wStr.data());
#else
    return ::dlopen(path.c_str(), RTLD_LOCAL | RTLD_LAZY);
#endif
}

void dlClose(void *handle)
{
#if GX_PLATFORM_WINDOWS
    ::FreeLibrary((HMODULE) handle);
#else
    ::dlclose(handle);
#endif
}

void *dlSym(void *handle, const std::string &symbol)
{
#if GX_PLATFORM_WINDOWS
    return (void *) ::GetProcAddress((HMODULE) handle, symbol.c_str());
#else
    return ::dlsym(handle, symbol.c_str());
#endif
}

std::string getEnv(const std::string &name)
{
#if GX_PLATFORM_WINDOWS
    const int32_t bufferSize = 512;
    std::vector<wchar_t> buffer(bufferSize);

    GWString wName(name);
    DWORD result = GetEnvironmentVariableW(wName.data(), buffer.data(), bufferSize);
    if (result == 0) {
        LogW("");
        return "";
    }
    if (bufferSize < result) {
        buffer.clear();
        buffer.resize(result);
        result = GetEnvironmentVariableW(wName.data(), buffer.data(), result);
        if (!result) {
            return "";
        }
    }
    return GString(buffer.data()).toStdString();
#else
    char *env = getenv(name.c_str());
    if (env) {
        return env;
    }
    return "";
#endif
}

static bool fillPath(GString &path, int depth = 0)
{
#if GX_PLATFORM_WINDOWS
    int32_t pIndex = path.indexOf("%");
    if (pIndex < 0) {
        return true;
    }
    if (depth >= 3) {
        return false;
    }

    int32_t pEnd = path.indexOf("%", pIndex + 1);
    if (pEnd < 0) {
        return true;
    }

    GString flag = path.substring(pIndex, pEnd - pIndex + 1);
    GString envKey = path.substring(pIndex + 1, pEnd - pIndex - 1);
    path = path.replace(flag, getEnv(envKey.toStdString()));
    return fillPath(path, depth + 1);
#else
    return true;
#endif
}

std::vector<std::string> getEnvPATH()
{
    GString PATH = getEnv("PATH");

#if GX_PLATFORM_WINDOWS
    std::string sep = ";";
#else
    std::string sep = ":";
#endif
    auto paths = PATH.split(sep);
    std::vector<std::string> ret;
    ret.reserve(paths.size());
    for (auto &p: paths) {
        if (!p.isEmpty() && fillPath(p)) {
            ret.push_back(p.toStdString());
        }
    }
    return ret;
}

std::string name()
{
#if GX_PLATFORM_WINDOWS
    return "Windows";
#elif GX_PLATFORM_LINUX
    return "Linux";
#elif GX_PLATFORM_OSX
    return "MacOS";
#elif GX_PLATFORM_IOS
    return "IOS";
#elif GX_PLATFORM_UWP
    return "UWP";
#elif GX_PLATFORM_ANDROID
    return "Android";
#elif GX_PLATFORM_BSD
    return "BSD";
#elif GX_PLATFORM_EMSCRIPTEN
    return "Web";
#else
    return "Unknown";
#endif
}

std::string cpuName()
{
#ifdef GX_CPU_NAME
    return GX_CPU_NAME;
#else
    return "Unknown";
#endif
}

std::string archName()
{
#ifdef GX_ARCH_NAME
    return GX_ARCH_NAME;
#else
    return "Unknown";
#endif
}

int execute(const std::string &cmd)
{
    return ::system(cmd.c_str());
}

}
GX_NS_END
