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

#ifndef GX_GUUID_H
#define GX_GUUID_H

#include "gobject.h"

#include "gx/common.h"


GX_NS_BEGIN

/**
 * @class GUuid
 * @brief Generate UUID and provide string output in multiple formats.
 */
class GX_API GUuid final
{
public:
    enum class FormatType : uint8_t
    {
        // 32 digits:
        // 00000000000000000000000000000000
        N,

        // 32 digits separated by hyphens:
        // 00000000-0000-0000-0000-000000000000
        D,

        // 32 digits separated by hyphens, enclosed in braces:
        // {00000000-0000-0000-0000-000000000000}
        B,

        // 32 digits separated by hyphens, enclosed in parentheses:
        // (00000000-0000-0000-0000-000000000000)
        P
    };

    union
    {
        struct
        {
            uint32_t a;
            uint32_t b;
            uint32_t c;
            uint32_t d;
        };

        uint8_t raw[16];
        uint32_t values[4];
    };
public:
    explicit GUuid();

    explicit GUuid(const std::string &uuid);

    GUuid(GUuid &&b) noexcept;

    GUuid(const GUuid &b);

public:
    std::string toString() const;

    std::string toString(FormatType formatType) const;

    bool isValid() const;

    bool operator==(const GUuid &rh) const;

    bool operator!=(const GUuid &rh) const;

    explicit operator bool() const
    {
        return isValid();
    }
};

GX_NS_END

namespace std
{

template<>
struct hash<gx::GUuid>
{
    size_t operator()(const gx::GUuid &type) const
    {
        return gx::hashOf(type);
    }
};
}

#endif //GX_GUUID_H
