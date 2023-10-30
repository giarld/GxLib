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

#include <gx/gversion.h>

#include "gx/gany.h"


GX_NS_BEGIN

void refGVersion()
{
    Class<GVersion>("Gx", "GVersion", "Gx version number conversion and comparison tool")
            .construct<uint16_t, uint16_t, uint16_t, uint16_t>()
            .construct<const std::string &>()
            .inherit<GObject>()
            .func("toUInt64", &GVersion::toUInt64)
            .func("getMajor", &GVersion::getMajor)
            .func("getMinor", &GVersion::getMinor)
            .func("getPatch", &GVersion::getPatch)
            .func("getBuild", &GVersion::getBuild)
            .func(MetaFunction::EqualTo, &GVersion::operator==)
            .func(MetaFunction::LessThan, &GVersion::operator<);
}

GX_NS_END
