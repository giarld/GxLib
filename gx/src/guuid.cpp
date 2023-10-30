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

#include "gx/guuid.h"

#include <gx/gtime.h>

#include <sstream>
#include <string>


#if GX_PLATFORM_WINDOWS

#include <objbase.h>

#elif GX_PLATFORM_LINUX || GX_PLATFORM_OSX || GX_PLATFORM_IOS

#include <uuid/uuid.h>

#else

#include <gx/gcrypto.h>

#endif

#define UUID_BUF_LEN 64

GX_NS_BEGIN

#if GX_PLATFORM_WINDOWS

static void genUUID(GUuid &result)
{
    static bool sInit = false;
    if (!sInit) {
        CoInitialize(nullptr);
        sInit = true;
    }

    CoCreateGuid(reinterpret_cast<GUID *>(&result));
}

#elif GX_PLATFORM_LINUX || GX_PLATFORM_OSX || GX_PLATFORM_IOS

static void genUUID(GUuid &result)
{
    uuid_t uuid;
    uuid_generate(uuid);
    auto ptr = (uint32_t*)&uuid;
    result.a = ptr[0];
    result.b = ptr[1];
    result.c = ptr[2];
    result.d = ptr[3];
}

#else

static void genUUID(GUuid &result)
{
    GByteArray randData = GCrypto::randomBytes(4*4);
    randData.read(result.a);
    randData.read(result.b);
    randData.read(result.c);
    randData.read(result.d);
}

#endif

GUuid::GUuid()
        : a(0), b(0), c(0), d(0)
{
    genUUID(*this);
}

GUuid::GUuid(const std::string &uuid)
{
    GString text = uuid;
    switch (uuid.size()) {
        case 32: {  // FormatType::N
            std::stringstream sA;
            std::stringstream sB;
            std::stringstream sC;
            std::stringstream sD;
            sA << std::hex << text.substring(0, 8);
            sB << std::hex << text.substring(8, 8);
            sC << std::hex << text.substring(16, 8);
            sD << std::hex << text.substring(24, 8);
            sA >> a;
            sB >> b;
            sC >> c;
            sD >> d;
        }
            break;
        case 36: {  // FormatType::D
            std::stringstream sA;
            std::stringstream sB;
            std::stringstream sC;
            std::stringstream sD;
            sA << std::hex << text.substring(0, 8);
            sB << std::hex << (text.substring(9, 4) + text.substring(14, 4));
            sC << std::hex << (text.substring(19, 4) + text.substring(24, 4));
            sD << std::hex << text.substring(28, 8);
            sA >> a;
            sB >> b;
            sC >> c;
            sD >> d;
        }
            break;
        case 38: { // FormatType::B or FormatType::P
            if (text.at(0) != text.at(text.length() - 1)) {
                std::stringstream sA;
                std::stringstream sB;
                std::stringstream sC;
                std::stringstream sD;
                sA << std::hex << text.substring(1, 8);
                sB << std::hex << (text.substring(10, 4) + text.substring(15, 4));
                sC << std::hex << (text.substring(20, 4) + text.substring(25, 4));
                sD << std::hex << text.substring(29, 8);
                sA >> a;
                sB >> b;
                sC >> c;
                sD >> d;
            }
        }
            break;
        default: {
            a = b = c = d = 0;
        }
    }
}

GUuid::GUuid(GUuid &&b) noexcept
        : a(b.a), b(b.b), c(b.c), d(b.d)
{
}

GUuid::GUuid(const GUuid &b)
        : a(b.a), b(b.b), c(b.c), d(b.d)
{
}

std::string GUuid::toString() const
{
    return toString(FormatType::D);
}

std::string GUuid::toString(GUuid::FormatType formatType) const
{
    char buf[UUID_BUF_LEN] = {0};

    switch (formatType) {
        case FormatType::N: {
            sprintf(buf,
                    "%08x%08x%08x%08x",
                    a, b, c, d);
        }
            break;
        case FormatType::B: {
            sprintf(buf,
                    "{%08x-%04x-%04x-%04x-%04x%08x}",
                    a, b >> 16, b & 0xFFFF, c >> 16, c & 0xFFFF, d);
        }
            break;
        case FormatType::P: {
            sprintf(buf,
                    "(%08x-%04x-%04x-%04x-%04x%08x)",
                    a, b >> 16, b & 0xFFFF, c >> 16, c & 0xFFFF, d);
        }
            break;
        default: {
            sprintf(buf,
                    "%08x-%04x-%04x-%04x-%04x%08x",
                    a, b >> 16, b & 0xFFFF, c >> 16, c & 0xFFFF, d);
        }
    }
    return buf;
}

bool GUuid::isValid() const
{
    return (a | b | c | d) != 0;
}

bool GUuid::operator==(const GUuid &rh) const
{
    return bitwiseEqual(values, rh.values);
}

bool GUuid::operator!=(const GUuid &rh) const
{
    return !(*this == rh);
}

GX_NS_END