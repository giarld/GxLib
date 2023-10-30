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

#include "ref_gx.h"

#include "gx/guuid.h"

#include "gx/enum.h"


GX_NS_BEGIN

DEF_ENUM_OPERATORS(GUuid::FormatType, uint8_t);

void refGUuid()
{
    Class<GUuid::FormatType>("Gx", "GUuidFormatType", "Enum GUuid::FormatType.")
            .defEnum("Enum", {
                    {"N", GUuid::FormatType::N},
                    {"D", GUuid::FormatType::D},
                    {"B", GUuid::FormatType::B},
                    {"P", GUuid::FormatType::P},
            })
            .func(MetaFunction::ToString, [](GUuid::FormatType &self) {
                switch (self) {
                    case GUuid::FormatType::N:
                        return "N";
                    case GUuid::FormatType::D:
                        return "D";
                    case GUuid::FormatType::B:
                        return "B";
                    case GUuid::FormatType::P:
                        return "P";
                }
                return "";
            })
                    REF_ENUM_OPERATORS(GUuid::FormatType);

    Class<GUuid>("Gx", "GUuid", "Gx uuid.")
            .construct<>()
            .construct<const std::string &>()
            .construct<GUuid &>()
            .func(MetaFunction::ToString, [](GUuid &self) {
                return self.toString();
            })
            .func(MetaFunction::EqualTo, &GUuid::operator==)
            .func("toString", [](GUuid &self, GUuid::FormatType &formatType) {
                return self.toString(formatType);
            })
            .func("isValid", &GUuid::isValid)
            .defEnum("FormatType", {
                    {"N", GUuid::FormatType::N},
                    {"D", GUuid::FormatType::D},
                    {"B", GUuid::FormatType::B},
                    {"P", GUuid::FormatType::P},
            });
}

GX_NS_END
