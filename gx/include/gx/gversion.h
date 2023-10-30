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

#ifndef GX_G_VERSION_H
#define GX_G_VERSION_H

#include "gobject.h"


GX_NS_BEGIN

/**
 * @class GVersion
 * @brief Version number conversion and comparison tool
 */
class GX_API GVersion : public GObject
{
public:
    explicit GVersion(uint16_t major, uint16_t minor, uint16_t patch, uint16_t build);

    explicit GVersion(const std::string &version);

public:
    uint64_t toUInt64() const;

    std::string toString() const override;

    uint16_t getMajor() const;

    uint16_t getMinor() const;

    uint16_t getPatch() const;

    uint16_t getBuild() const;

public:
    bool operator==(const GVersion &b) const;

    bool operator!=(const GVersion &b) const;

    bool operator<(const GVersion &b) const;

    bool operator<=(const GVersion &b) const;

    bool operator>(const GVersion &b) const;

    bool operator>=(const GVersion &b) const;

private:
    uint16_t mMajor = 0;
    uint16_t mMinor = 0;
    uint16_t mPatch = 0;
    uint16_t mBuild = 0;
};

GX_NS_END

#endif //GX_G_VERSION_H
