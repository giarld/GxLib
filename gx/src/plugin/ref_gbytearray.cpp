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

#include "gx/gbytearray.h"


GX_NS_BEGIN

void refGByteArray()
{
    Class<GByteArray>("Gx", "GByteArray", "Gx byte array")
            .construct<>()
            .construct<uint32_t>()
            .construct<const uint8_t *, int32_t>()
            .construct<const std::vector<uint8_t> &>()
            .construct<const GByteArray &>()
            .inherit<GObject>()
            .defEnum("ByteOrder", {
                    {"BigEndian",    GByteArray::BigEndian},
                    {"LittleEndian", GByteArray::LittleEndian}
            }, "Byte order type.")
            .defEnum("SeekMode", {
                    {"Set", (int) SEEK_SET},
                    {"Cur", (int) SEEK_CUR},
                    {"End", (int) SEEK_END}
            }, "Seek mode")
            .func("setByteOrder", &GByteArray::setByteOrder)
            .func("reset", [](GByteArray &self) {
                self.reset();
            }, "Reset read and write position.")
            .func("reset", [](GByteArray &self, uint32_t size) {
                self.reset(size);
            }, "Reset byte array with size, and reset read and write position.")
            .func("capacity", &GByteArray::capacity, "Get capacity.")
            .func("size", &GByteArray::size, "Get size(write size).")
            .func("isEmpty", &GByteArray::isEmpty, "Check if byte array is empty.")
            .func("data", &GByteArray::data, "Get data pointer.")
            .func("clear", &GByteArray::clear, "Clear byte array.")
            .func("write", [](GByteArray &self, GAnyConstBytePtr data, uint32_t size) {
                self.write(data, size);
            }, "Write data to byte array. arg1: data; arg2: size.")
            .func("writeInt8", [](GByteArray &self, GAny &data) {
                self.write((int8_t) data.toInt32());
            }, "Write int8 to byte array.")
            .func("writeUInt8", [](GByteArray &self, GAny &data) {
                self.write((uint8_t) data.toInt32());
            }, "Write uint8 to byte array.")
            .func("writeInt16", [](GByteArray &self, GAny &data) {
                self.write((int16_t) data.toInt32());
            }, "Write int16 to byte array.")
            .func("writeUInt16", [](GByteArray &self, GAny &data) {
                self.write((uint16_t) data.toInt32());
            }, "Write uint16 to byte array.")
            .func("writeInt32", [](GByteArray &self, GAny &data) {
                self.write((int32_t) data.toInt32());
            }, "Write int32 to byte array.")
            .func("writeUInt32", [](GByteArray &self, GAny &data) {
                self.write((uint32_t) data.toInt32());
            }, "Write uint32 to byte array.")
            .func("writeInt64", [](GByteArray &self, GAny &data) {
                self.write((int64_t) data.toInt64());
            }, "Write int64 to byte array.")
            .func("writeUInt64", [](GByteArray &self, GAny &data) {
                self.write((uint64_t) data.toInt64());
            }, "Write uint64 to byte array.")
            .func("writeBool", [](GByteArray &self, GAny &data) {
                self.write(data.toBool());
            }, "Write bool to byte array.")
            .func("writeFloat", [](GByteArray &self, GAny &data) {
                self.write(data.toFloat());
            }, "Write float to byte array.")
            .func("writeDouble", [](GByteArray &self, GAny &data) {
                self.write(data.toDouble());
            }, "Write double to byte array.")
            .func("writeString", [](GByteArray &self, const std::string &data) {
                self.write(data);
            }, "Write string to byte array.")
            .func("writeStringData", [](GByteArray &self, const std::string &data) {
                self.write(data.data(), data.size());
            }, "Write a string as a block to byte array.")
            .func("writeBytes", [](GByteArray &self, const GByteArray &data) {
                self.write(data);
            }, "Write GByteArray to byte array.")
            .func("writeGAny", [](GByteArray &self, const GAny &data) {
                self.write(data);
            }, "Write GAny to byte array.")
            .func("read", [](GByteArray &self, GAnyBytePtr data, uint32_t size) {
                self.read(data, size);
            }, "Read data from byte array. arg1: data; arg2: size.")
            .func("readInt8", [](GByteArray &self) {
                int8_t v;
                self.read(v);
                return v;
            }, "Read int8 from byte array.")
            .func("readUInt8", [](GByteArray &self) {
                uint8_t v;
                self.read(v);
                return v;
            }, "Read uint8 from byte array.")
            .func("readInt16", [](GByteArray &self) {
                int16_t v;
                self.read(v);
                return v;
            }, "Read int16 from byte array.")
            .func("readUInt16", [](GByteArray &self) {
                uint16_t v;
                self.read(v);
                return v;
            }, "Read uint16 from byte array.")
            .func("readInt32", [](GByteArray &self) {
                int32_t v;
                self.read(v);
                return v;
            }, "Read int32 from byte array.")
            .func("readUInt32", [](GByteArray &self) {
                uint32_t v;
                self.read(v);
                return v;
            }, "Read uint32 from byte array.")
            .func("readInt64", [](GByteArray &self) {
                int64_t v;
                self.read(v);
                return v;
            }, "Read int64 from byte array.")
            .func("readUInt64", [](GByteArray &self) {
                uint64_t v;
                self.read(v);
                return v;
            }, "Read uint64 from byte array.")
            .func("readBool", [](GByteArray &self) {
                bool v;
                self.read(v);
                return v;
            }, "Read bool from byte array.")
            .func("readFloat", [](GByteArray &self) {
                float v;
                self.read(v);
                return v;
            }, "Read float from byte array.")
            .func("readDouble", [](GByteArray &self) {
                double v;
                self.read(v);
                return v;
            }, "Read double from byte array.")
            .func("readString", [](GByteArray &self) {
                std::string v;
                self.read(v);
                return v;
            }, "Read string from byte array.")
            .func("readStringData", [](GByteArray &self, int32_t len) {
                std::vector<char> buffer(len);
                self.read(buffer.data(), len);
                return std::string(buffer.data(), len);
            }, "Read a string as a block from byte array.")
            .func("readBytes", [](GByteArray &self) {
                GByteArray v;
                self.read(v);
                return v;
            }, "Read GByteArray from byte array.")
            .func("readGAny", [](GByteArray &self) {
                GAny v;
                self.read(v);
                return v;
            }, "Read GAny from byte array.")
            .func("seekWritePos", &GByteArray::seekWritePos, "Seek write position. arg1: mode(SeekMode); arg2: size.")
            .func("seekReadPos", &GByteArray::seekReadPos, "Seek read position. arg1: mode(SeekMode); arg2: size.")
            .func("writePos", &GByteArray::writePos, "Get write position.")
            .func("readPos", &GByteArray::readPos, "Get read position.")
            .func("canReadMore", &GByteArray::canReadMore, "Check if can read more.")
            .func("compare", &GByteArray::compare, "Compare two byte array.")
            .func("toHexString", [](GByteArray &self) {
                return self.toHexString();
            }, "Convert to hex string.")
            .func("toHexString", [](GByteArray &self, bool uppercase) {
                return self.toHexString(uppercase);
            }, "Convert to hex string. arg1: uppercase.")
            .func(MetaFunction::EqualTo, [](GByteArray &self, GByteArray &other) {
                return self == other;
            })
            .staticFunc("fromHexString", &GByteArray::fromHexString, "Create from hex string. arg1: hexString.")
            .staticFunc("compress", [](const uint8_t *data, uint32_t dataSize) {
                return GByteArray::compress(data, dataSize);
            }, "Compress data. arg1: data, dataSize.")
            .staticFunc("compress", [](const GByteArray &in) {
                return GByteArray::compress(in);
            }, "Compress data.")
            .staticFunc("isCompressed", &GByteArray::isCompressed)
            .staticFunc("uncompress", [](const uint8_t *data, uint32_t dataSize, uint32_t uncompSize) {
                return GByteArray::uncompress(data, dataSize, uncompSize);
            }, "Uncompress data. arg1: data; arg2: dataSize; arg3: uncompSize.")
            .staticFunc("uncompress", [](const GByteArray &in) {
                return GByteArray::uncompress(in);
            }, "Uncompress data.")
            .staticFunc("base64Encode", &GByteArray::base64Encode, "Base64 encode data.")
            .staticFunc("base64Decode", &GByteArray::base64Decode, "Base64 decode data.")
            .staticFunc("md5Sum", &GByteArray::md5Sum, "Get md5 sum of data.")
            .staticFunc("sha1Sum", &GByteArray::sha1Sum, "Get sha1 sum of data.")
            .staticFunc("sha256Sum", &GByteArray::sha256Sum, "Get sha256 sum of data.");
}

GX_NS_END
