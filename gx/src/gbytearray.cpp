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

#include "gx/gbytearray.h"

#include <gx/gglobal_memory_pool.h>
#include <gx/ghash_sum.h>
#include <gx/gany.h>
#include <gx/gstring.h>
#include <gx/debug.h>

#include <memory.h>
#include <cstring>
#include <algorithm>
#include <iomanip>
#include <sstream>

#include <zlib.h>


GX_NS_BEGIN

static const char sCompFlag[4] = {
        '_',
        'C',
        'M',
        'P'
};

static const std::string sBase64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

static bool uncompressBlob(const uint8_t *src, uint32_t srcLen, uint8_t *dst, uint32_t *dstLen)
{
    uLongf tempDstLen = *dstLen;
    int ret = uncompress(dst, &tempDstLen, src, srcLen);
    *dstLen = tempDstLen;

    return ret == Z_OK;
}

static inline bool isBase64(uint8_t c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

void writeGAnyToByteArray(GByteArray &ba, const GAny &obj);

GAny readGAnyFromByteArray(const GByteArray &ba);

/// ================ GByteArray ================

GByteArray::GByteArray(uint32_t size)
{
    reset(std::max(size, 1u));
}

GByteArray::GByteArray(const uint8_t *data, int32_t size)
{
    if (size < 0) {
        size = (int32_t) strlen((const char *) data) + 1;
    }
    reset(size);
    write(data, size);
}

GByteArray::GByteArray(const std::vector<uint8_t> &data)
        : GByteArray(data.data(), (int32_t) data.size())
{
}

GByteArray::GByteArray(GByteArray &&other) noexcept
{
    std::swap(mWritePos, other.mWritePos);
    std::swap(mReadPos, other.mReadPos);
    std::swap(mByteOrder, other.mByteOrder);
    std::swap(mBufferRef, other.mBufferRef);
}

GByteArray::GByteArray(const GByteArray &other)
{
    mWritePos = other.mWritePos;
    mReadPos = other.mReadPos;
    mByteOrder = other.mByteOrder;
    mBufferRef = other.mBufferRef;
}

GByteArray::~GByteArray()
{
    mBufferRef = nullptr;
}

GByteArray &GByteArray::operator=(const GByteArray &b)
{
    if (this != &b) {
        mWritePos = b.mWritePos;
        mReadPos = b.mReadPos;
        mByteOrder = b.mByteOrder;
        mBufferRef = b.mBufferRef;
    }
    return *this;
}

GByteArray &GByteArray::operator=(GByteArray &&b) noexcept
{
    if (this != &b) {
        std::swap(mWritePos, b.mWritePos);
        std::swap(mReadPos, b.mReadPos);
        std::swap(mByteOrder, b.mByteOrder);
        std::swap(mBufferRef, b.mBufferRef);
    }
    return *this;
}

void GByteArray::setByteOrder(GByteArray::ByteOrder byteOrder)
{
    mByteOrder = byteOrder;
}

void GByteArray::reset(uint32_t size)
{
    if (size > 0) {
        resize(size);
    }
    mWritePos = mReadPos = 0;
}

uint32_t GByteArray::capacity() const
{
    return mBufferRef->size;
}

uint32_t GByteArray::size() const
{
    return mWritePos;
}

bool GByteArray::isEmpty() const
{
    return size() == 0;
}

const uint8_t *GByteArray::data() const
{
    return (const uint8_t *) mBufferRef->buffer;
}

void GByteArray::clear()
{
    mWritePos = mReadPos = 0;
}

void GByteArray::write(const void *data, uint32_t size)
{
    copyOnWrite();

    uint32_t outSize = mWritePos + size;
    if (outSize > mBufferRef->size) {
        auto newSize = (uint32_t) ((float) mBufferRef->size * 1.5f);
        if (newSize < outSize) {
            newSize = (uint32_t) ((float) outSize * 1.5f);
        }
        resize(newSize);
    }
    uint8_t *p = mBufferRef->buffer + mWritePos;
    memcpy(p, data, size);
    byteOrder(p, size);

    mWritePos += size;
}

void GByteArray::write(const std::string &in)
{
    write((uint32_t) in.size());
    write(in.data(), in.size());
}

void GByteArray::write(const GString &in)
{
    write((uint32_t) in.count());
    write(in.data(), in.count());
}

void GByteArray::write(const GByteArray &in)
{
    write((uint32_t) in.size());
    write(in.data(), in.size());
}

void GByteArray::write(const std::vector<uint8_t> &in)
{
    write((uint32_t) in.size());
    write(in.data(), in.size());
}

void GByteArray::write(const GAny &any)
{
    GByteArray pack;
    writeGAnyToByteArray(pack, any);
    write(pack);
}

void GByteArray::read(void *data, uint32_t size) const
{
    GX_ASSERT_S(mReadPos + size <= mWritePos, "GByteArray::read error (position: %d - %d, size: %d)",
                mReadPos, mReadPos + size, mWritePos);

    if (mReadPos + size > mWritePos) {
        size = mWritePos - mReadPos;
    }
    if (size == 0) {
        return;
    }

    memcpy(data, mBufferRef->buffer + mReadPos, size);
    byteOrder((uint8_t *) data, size);

    mReadPos += size;
}

void GByteArray::read(std::string &out) const
{
    uint32_t size;
    read(size);
    out.resize(size);
    read(out.data(), size);
}

void GByteArray::read(GString &out) const
{
    std::string temp;
    read(temp);
    out = temp;
}

void GByteArray::read(GByteArray &out) const
{
    uint32_t size;
    this->read(size);
    out.reset(size);
    this->read(out.ptr(), size);
    out.seekWritePos(SEEK_SET, (int32_t) size);
}

void GByteArray::read(std::vector<uint8_t> &out) const
{
    uint32_t size;
    this->read(size);
    out.resize(size);
    this->read(out.data(), size);
}

void GByteArray::read(GAny &any) const
{
    GByteArray pack;
    read(pack);
    any = readGAnyFromByteArray(pack);
}

void GByteArray::seekWritePos(int mode, int32_t size)
{
    int32_t pos;
    switch (mode) {
        case SEEK_SET:
            pos = size;
            break;
        default:
        case SEEK_CUR:
            pos = mWritePos + size;
            break;
        case SEEK_END:
            pos = mBufferRef->size + size;
            break;
    }

    GX_ASSERT_S(pos >= 0 && pos <= mBufferRef->size,
                "GByteArray::seekWritePos error (pos: %d out range(0-%d))", pos, mBufferRef->size);

    if (pos < 0) {
        pos = 0;
    }
    if (pos > mBufferRef->size) {
        pos = (int32_t) mBufferRef->size;
    }
    mWritePos = pos;
}

void GByteArray::seekReadPos(int mode, int32_t size) const
{
    int32_t pos;
    switch (mode) {
        case SEEK_SET:
            pos = size;
            break;
        default:
        case SEEK_CUR:
            pos = mReadPos + size;
            break;
        case SEEK_END:
            pos = mBufferRef->size + size;
            break;
    }

    GX_ASSERT_S(pos >= 0 && pos <= mBufferRef->size,
                "GByteArray::seekReadPos error (pos: %d out range(0-%d))", pos, mBufferRef->size);

    if (pos < 0) {
        pos = 0;
    }
    if (pos > mBufferRef->size) {
        pos = (int32_t) mBufferRef->size;
    }
    mReadPos = pos;
}

uint32_t GByteArray::writePos() const
{
    return mWritePos;
}

uint32_t GByteArray::readPos() const
{
    return mReadPos;
}

bool GByteArray::canReadMore() const
{
    return mReadPos < mWritePos;
}

bool GByteArray::compare(const GByteArray &other) const
{
    return size() == other.size() && memcmp(data(), other.data(), size()) == 0;
}

std::string GByteArray::toHexString(bool uppercase) const
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    if (uppercase) {
        ss << std::uppercase;
    }
    for (uint32_t i = 0; i < size(); i++) {
        ss << std::setw(2) << (int) mBufferRef->buffer[i];
    }
    return ss.str();
}

std::string GByteArray::toString() const
{
    std::stringstream ss;
    ss << "<GByteArray at " << std::hex << this << std::dec << ""
       << ", size: " << size()
       << ", md5: " << GByteArray::md5Sum(*this).toHexString()
       << ">";
    return ss.str();
}

GByteArray GByteArray::fromHexString(const std::string &hexString)
{
    // Convert the string to lowercase first
    std::string str = hexString;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);

    // Determine whether a string is legal
    if (str.size() % 2 != 0) {
        return GByteArray();
    }

    // Convert a string to a byte array
    GByteArray bytes(str.size() / 2);
    for (uint32_t i = 0; i < str.size(); i += 2) {
        char c1 = str[i];
        char c2 = str[i + 1];
        if (c1 >= '0' && c1 <= '9') {
            c1 -= '0';
        } else if (c1 >= 'a' && c1 <= 'f') {
            c1 -= 'a' - 10;
        } else {
            return GByteArray();
        }
        if (c2 >= '0' && c2 <= '9') {
            c2 -= '0';
        } else if (c2 >= 'a' && c2 <= 'f') {
            c2 -= 'a' - 10;
        } else {
            return GByteArray();
        }
        bytes.write((char) ((c1 << 4) | c2));
    }
    return bytes;
}

std::vector<uint8_t> GByteArray::compress(const uint8_t *data, uint32_t dataSize)
{
    uLongf preDstLen = compressBound(dataSize);

    std::vector<uint8_t> buffer(preDstLen);
    if (::compress(buffer.data(), &preDstLen, data, dataSize) == Z_OK) {
        buffer.resize(preDstLen);
        return buffer;
    }

    return {};
}

GByteArray GByteArray::compress(const GByteArray &in)
{
    if (in.isEmpty()) {
        return GByteArray();
    }

    if (isCompressed(in)) {
        return GByteArray();
    }

    GByteArray out;
    out.write(sCompFlag, sizeof(sCompFlag));
    out.write((uint32_t) in.size());

    auto comp = GByteArray::compress(in.data(), in.size());
    out.write(comp.data(), comp.size());

    return out;
}

bool GByteArray::isCompressed(const GByteArray &data)
{
    if (data.size() < sizeof(sCompFlag)) {
        return false;
    }

    char flagBuf[4];
    memcpy(flagBuf, data.data(), sizeof(sCompFlag));

    return memcmp(sCompFlag, flagBuf, sizeof(sCompFlag)) == 0;
}

std::vector<uint8_t> GByteArray::uncompress(const uint8_t *data, uint32_t dataSize, uint32_t uncompSize)
{
    std::vector<uint8_t> buffer(uncompSize);
    uint32_t total = uncompSize;

    if (uncompressBlob(data, dataSize, buffer.data(), &total)) {
        return buffer;
    }
    return {};
}

GByteArray GByteArray::uncompress(const GByteArray &in)
{
    if (!isCompressed(in)) {
        return GByteArray();
    }
    auto oldReadPos = (int32_t) in.mReadPos;
    in.seekReadPos(SEEK_CUR, sizeof(sCompFlag));
    uint32_t uncompSize;
    in.read(uncompSize);

    GByteArray out(uncompress(in.data() + in.readPos(), in.size() - in.readPos(), uncompSize));
    in.seekReadPos(SEEK_SET, oldReadPos);
    return out;
}

std::string GByteArray::base64Encode(const GByteArray &data)
{
    std::string ret;

    int i = 0;
    int j = 0;

    uint8_t char_array_3[3];
    uint8_t char_array_4[4];

    const uint8_t *buf = data.data();
    uint32_t bufLen = data.size();

    while (bufLen--) {
        char_array_3[i++] = *(buf++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++) {
                ret += sBase64Chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++) {
            ret += sBase64Chars[char_array_4[j]];
        }

        while ((i++ < 3)) {
            ret += '=';
        }
    }

    return ret;
}

GByteArray GByteArray::base64Decode(const std::string &codes)
{
    int in_len = (int) codes.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    uint8_t char_array_4[4], char_array_3[3];

    std::vector<uint8_t> ret;

    while (in_len-- && (codes[in_] != '=') && isBase64(codes[in_])) {
        char_array_4[i++] = codes[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = sBase64Chars.find((char) char_array_4[i]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++) {
                ret.push_back(char_array_3[i]);
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }

        for (j = 0; j < 4; j++) {
            char_array_4[j] = sBase64Chars.find((char) char_array_4[j]);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) { ret.push_back(char_array_3[j]); }
    }

    return GByteArray(ret);
}

GByteArray GByteArray::md5Sum(const GByteArray &data)
{
    auto hashSum = GHashSum::hashSum(GHashSum::Md5);
    hashSum->update(data);
    return hashSum->final();
}

GByteArray GByteArray::sha1Sum(const GByteArray &data)
{
    auto hashSum = GHashSum::hashSum(GHashSum::Sha1);
    hashSum->update(data);
    return hashSum->final();
}

GByteArray GByteArray::sha256Sum(const GByteArray &data)
{
    auto hashSum = GHashSum::hashSum(GHashSum::Sha256);
    hashSum->update(data);
    return hashSum->final();
}


void GByteArray::resize(uint32_t size)
{
    if (mBufferRef) {
        if (size > mBufferRef->size) {
            copyOnWrite();
            mBufferRef->resize(size);
        }
    } else {
        mBufferRef = std::make_shared<BufferRef>(size);
    }
}

uint8_t *GByteArray::ptr()
{
    copyOnWrite();
    return (uint8_t *) mBufferRef->buffer;
}

void GByteArray::align(uint32_t *pos, uint32_t alignment)
{
    uint32_t mask = alignment - 1;
    uint32_t xpos = ((*pos) + mask) & (~mask);
    *pos = xpos;
}

void GByteArray::byteOrder(uint8_t *data, uint32_t len) const
{
    static ByteOrder systemOrder =
#if GX_CPU_ENDIAN_LITTLE
     ByteOrder::LittleEndian;
#else
    ByteOrder::BigEndian;
#endif

    if (mByteOrder != systemOrder) {
        std::reverse(data, data + len);
    }
}

void GByteArray::copyOnWrite()
{
    if (mBufferRef.use_count() <= 1) {
        return;
    }

    auto newBufferRef = std::make_shared<BufferRef>(mBufferRef->size);
    memcpy(newBufferRef->buffer, mBufferRef->buffer, mBufferRef->size);

    mBufferRef = newBufferRef;
}

GByteArray::BufferRef::BufferRef(uint32_t _size)
{
    size = _size;
    buffer = (uint8_t *) GGlobalMemoryPool::alloc(size);
}

GByteArray::BufferRef::~BufferRef()
{
    GGlobalMemoryPool::free(buffer, size);
}

void GByteArray::BufferRef::resize(uint32_t newSize)
{
    if (newSize <= this->size) {
        return;
    }
    auto *newBuffer = (uint8_t *) GGlobalMemoryPool::alloc(newSize);
    memcpy(newBuffer, this->buffer, std::min(this->size, newSize));
    GGlobalMemoryPool::free(this->buffer, this->size);
    this->buffer = newBuffer;
    this->size = newSize;
}

/// ================ GAny ================

void writeGAnyObjectToByteArray(GByteArray &ba, const GAny &obj)
{
    if (!obj.isObject()) {
        return;
    }

    const auto &tObj = obj.unsafeAs<std::unordered_map<std::string, GAny>>();
    int32_t size = (int32_t) obj.size();

    ba.write(size);
    for (const auto &it: tObj) {
        ba.write(it.first);
        writeGAnyToByteArray(ba, it.second);
    }
}

void writeGAnyArrayToByteArray(GByteArray &ba, const GAny &obj)
{
    if (!obj.isArray()) {
        return;
    }

    const auto &vec = obj.unsafeAs<std::vector<GAny>>();
    int32_t size = (int32_t) vec.size();

    ba.write(size);
    for (const auto &it: vec) {
        writeGAnyToByteArray(ba, it);
    }
}

GAny readGAnyObjectFromByteArray(const GByteArray &value)
{
    int32_t size;
    value.read(size);

    GAny obj = GAny::object();
    for (int32_t i = 0; i < size; i++) {
        std::string key;
        value.read(key);
        obj[key] = readGAnyFromByteArray(value);
    }
    return obj;
}

GAny readGAnyArrayFromByteArray(const GByteArray &value)
{
    int32_t size;
    value.read(size);

    GAny obj = GAny::array();
    for (int32_t i = 0; i < size; i++) {
        obj.pushBack(readGAnyFromByteArray(value));
    }
    return obj;
}

void writeGAnyToByteArray(GByteArray &ba, const GAny &obj)
{
    switch (obj.type()) {
        case AnyType::undefined_t: {
            ba.write((uint8_t) 0);
        }
            break;
        case AnyType::null_t: {
            ba.write((uint8_t) 1);
        }
            break;
        case AnyType::boolean_t: {
            ba.write((uint8_t) 2);
            ba.write(obj.as<bool>());
        }
            break;
        case AnyType::int32_t: {
            if (obj.is<int32_t>()) {
                ba.write((uint8_t) 3);
                ba.write(obj.unsafeAs<int32_t>());
            } else {
                ba.write((uint8_t) 4);
                ba.write(obj.unsafeAs<uint32_t>());
            }
        }
            break;
        case AnyType::int64_t: {
            if (obj.is<int64_t>()) {
                ba.write((uint8_t) 5);
                ba.write(obj.unsafeAs<int64_t>());
            } else {
                ba.write((uint8_t) 6);
                ba.write(obj.unsafeAs<uint64_t>());
            }
        }
            break;
        case AnyType::float_t: {
            ba.write((uint8_t) 7);
            ba.write(obj.as<float>());
        }
            break;
        case AnyType::double_t: {
            ba.write((uint8_t) 8);
            ba.write(obj.as<double>());
        }
            break;
        case AnyType::string_t: {
            ba.write((uint8_t) 9);
            ba.write(obj.as<std::string>());
        }
            break;
        case AnyType::array_t: {
            ba.write((uint8_t) 10);
            writeGAnyArrayToByteArray(ba, obj);
        }
            break;
        case AnyType::object_t: {
            ba.write((uint8_t) 11);
            writeGAnyObjectToByteArray(ba, obj);
        }
            break;
        case AnyType::class_t: {
            ba.write((uint8_t) 12);
            std::stringstream ss;
            auto &cl = obj.as<GAnyClass>();
            ss << "<Class: " << cl.getName() << ">";
            ba.write(ss.str());
        }
            break;
        default: {
            if (obj.is<int8_t>()) {
                ba.write((uint8_t) 13);
                ba.write(obj.unsafeAs<int8_t>());
            } else if (obj.is<uint8_t>()) {
                ba.write((uint8_t) 14);
                ba.write(obj.unsafeAs<uint8_t>());
            } else if (obj.is<int16_t>()) {
                ba.write((uint8_t) 15);
                ba.write(obj.unsafeAs<int16_t>());
            } else if (obj.is<uint16_t>()) {
                ba.write((uint8_t) 16);
                ba.write(obj.unsafeAs<uint16_t>());
            } else if (obj.is<GByteArray>()) {
                ba.write((uint8_t) 17);
                ba.write(obj.as<GByteArray>());
            } else {
                ba.write((uint8_t) 254);
                std::stringstream ss;
                ss << "<" << obj.classTypeName() << " at " << obj.value().get() << ">";
                ba.write(ss.str());
            }
        }
            break;
    }
}

GAny readGAnyFromByteArray(const GByteArray &ba)
{
    uint8_t type;
    ba.read(type);

    switch (type) {
        default:
        case 0:
            return GAny::undefined();
        case 1:
            return GAny::null();
        case 2: {
            bool value;
            ba.read(value);
            return value;
        }
        case 3: {
            int32_t value;
            ba.read(value);
            return value;
        }
        case 4: {
            uint32_t value;
            ba.read(value);
            return value;
        }
        case 5: {
            int64_t value;
            ba.read(value);
            return value;
        }
        case 6: {
            uint64_t value;
            ba.read(value);
            return value;
        }
        case 7: {
            float value;
            ba.read(value);
            return value;
        }
        case 8: {
            double value;
            ba.read(value);
            return value;
        }
        case 9: {
            std::string value;
            ba.read(value);
            return value;
        }
        case 10: {
            GAny obj = readGAnyArrayFromByteArray(ba);
            return obj;
        }
        case 11: {
            GAny obj = readGAnyObjectFromByteArray(ba);
            return obj;
        }
        case 13: {
            int8_t value;
            ba.read(value);
            return value;
        }
        case 14: {
            uint8_t value;
            ba.read(value);
            return value;
        }
        case 15: {
            int16_t value;
            ba.read(value);
            return value;
        }
        case 16: {
            uint16_t value;
            ba.read(value);
            return value;
        }
        case 17: {
            GByteArray value;
            ba.read(value);
            return value;
        }
        case 12:
        case 254: {
            std::string value;
            ba.read(value);
            return GAny::create(value);
        }
    }
}

GX_NS_END