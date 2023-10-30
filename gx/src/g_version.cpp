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

#include "gx/gversion.h"

#include "gx/gstring.h"

#include <sstream>


GX_NS_BEGIN

GVersion::GVersion(uint16_t major, uint16_t minor, uint16_t patch, uint16_t build)
        : mMajor(major), mMinor(minor), mPatch(patch), mBuild(build)
{
}

GVersion::GVersion(const std::string &version)
{
    GString verStr(version);
    auto verPs = verStr.split(".");

    if (!verPs.empty()) {
        mMajor = std::stoi(verPs[0].toStdString());
    }
    if (verPs.size() >= 2) {
        mMinor = std::stoi(verPs[1].toStdString());
    }
    if (verPs.size() >= 3) {
        mPatch = std::stoi(verPs[2].toStdString());
    }
    if (verPs.size() >= 4) {
        mBuild = std::stoi(verPs[3].toStdString());
    }
}

uint64_t GVersion::toUInt64() const
{
    return mBuild | (mPatch << 16) | ((uint64_t) mMinor << 32) | ((uint64_t) mMajor << 48);
}

std::string GVersion::toString() const
{
    std::stringstream ss;
    ss << mMajor << "." << mMinor << "." << mPatch << "." << mBuild;
    return ss.str();
}

uint16_t GVersion::getMajor() const
{
    return mMajor;
}

uint16_t GVersion::getMinor() const
{
    return mMinor;
}

uint16_t GVersion::getPatch() const
{
    return mPatch;
}

uint16_t GVersion::getBuild() const
{
    return mBuild;
}

bool GVersion::operator==(const GVersion &b) const
{
    return this->toUInt64() == b.toUInt64();
}

bool GVersion::operator!=(const GVersion &b) const
{
    return this->toUInt64() != b.toUInt64();
}

bool GVersion::operator<(const GVersion &b) const
{
    return this->toUInt64() < b.toUInt64();
}

bool GVersion::operator<=(const GVersion &b) const
{
    return this->toUInt64() <= b.toUInt64();
}

bool GVersion::operator>(const GVersion &b) const
{
    return this->toUInt64() > b.toUInt64();
}

bool GVersion::operator>=(const GVersion &b) const
{
    return this->toUInt64() >= b.toUInt64();
}

GX_NS_END
