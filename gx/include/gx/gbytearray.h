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

#ifndef GX_GBYTEARRAY_H
#define GX_GBYTEARRAY_H

#include "gobject.h"

#include <vector>
#include <string>
#include <memory>


GX_NS_BEGIN

class GAny;

class GString;

/**
 * @class GByteArray
 * @brief Byte array class, providing operations such as read and write, HASH calculation,
 * compression and decompression, base64 encoding and decoding for continuous binary data
 */
class GX_API GByteArray final : public GObject
{
public:
    enum ByteOrder
    {
        BigEndian,
        LittleEndian
    };

public:
    explicit GByteArray(uint32_t size = 0);

    explicit GByteArray(const uint8_t *data, int32_t size = -1);

    explicit GByteArray(const std::vector<uint8_t> &data);

    GByteArray(GByteArray &&other) noexcept;

    GByteArray(const GByteArray &other);

    ~GByteArray();

    GByteArray &operator=(const GByteArray &b);

    GByteArray &operator=(GByteArray &&b) noexcept;

public:
    void setByteOrder(ByteOrder byteOrder);

    /**
     * @brief Reset<br>
     * When size is 0, only reset the write and read pointers. When size is not 0,
     * reset the buffer size and reset the write and read pointers.
     * @param size
     */
    void reset(uint32_t size = 0);

    /**
     * @brief Obtain buffer capacity
     * @return
     */
    uint32_t capacity() const;

    /**
     * @brief Obtain the size of the write range.
     * @return
     */
    uint32_t size() const;

    /**
     * @brief Determine if it is empty
     * @note
     * size() == 0
     *
     * @return
     */
    bool isEmpty() const;

    /**
     * @brief Get buffer header pointer
     * @return
     */
    const uint8_t *data() const;

    /**
     * @brief Reset the read/write pointer to 0
     */
    void clear();

    /**
     * @brief Write data
     * @param data data pointer
     * @param size data size
     */
    void write(const void *data, uint32_t size);

    /**
     * @brief Write data
     * @tparam Type data type
     * @param in    data
     */
    template<typename Type>
    void write(const Type &in)
    {
//        align(&mWritePos, __alignof(Type));
        write(reinterpret_cast<const unsigned char *>(&in), sizeof(Type));
    }

    /**
     * @brief Write string
     * @param in
     */
    void write(const std::string &in);

    /**
     * @brief Write string
     * @param in
     */
    void write(const GString &in);

    /**
     * @brief Write a GByteArray
     * @param in
     */
    void write(const GByteArray &in);

    /**
     * @brief Write data
     * @param in
     */
    void write(const std::vector<uint8_t> &in);

    void write(const GAny &any);

    /**
     * @brief Read data
     * @param data
     * @param size
     */
    void read(void *data, uint32_t size) const;

    /**
     * @brief Read data
     * @tparam Type type of output
     * @param out   output data
     */
    template<typename Type>
    void read(Type &out) const
    {
//        align(&mReadPos, __alignof(Type));
        read(reinterpret_cast<unsigned char *>(&out), sizeof(Type));
    }

    /**
     * @brief Read String
     * @param out
     */
    void read(std::string &out) const;

    /**
     * @brief Read String
     * @param out
     */
    void read(GString &out) const;

    /**
     * @brief Reading a GByteArray
     * @param out
     */
    void read(GByteArray &out) const;

    /**
     * @brief Read data
     * @param out
     */
    void read(std::vector<uint8_t> &out) const;

    void read(GAny &any) const;

    friend std::ostream &operator<<(std::ostream &os, const GByteArray &out)
    {
        os << out.size();
        os.write((const char *) out.data(), out.size());

        return os;
    }

    friend std::istream &operator>>(std::istream &is, GByteArray &in)
    {
        uint32_t size;
        is >> size;

        char *temp = new char[size];
        is.read(temp, size);

        in.reset(size);
        in.write(temp, size);

        delete[] temp;
        return is;
    }

    /**
     * @brief Seek write position
     *
     * @param mode SEEK_SET/SEEK_CUR/SEEK_END
     * @param size Positive and negative values represent direction, positive to right, negative to left
     */
    void seekWritePos(int mode, int32_t size);

    /**
     * @brief Seek read position
     *
     * @param mode SEEK_SET/SEEK_CUR/SEEK_END
     * @param size Positive and negative values represent direction, positive to right, negative to left
     */
    void seekReadPos(int mode, int32_t size) const;

    /**
     * @brief Get write pointer position
     * @return
     */
    uint32_t writePos() const;

    /**
     * @brief Get read pointer position
     * @return
     */
    uint32_t readPos() const;

    /**
     * @brief Can read more
     * @return
     */
    bool canReadMore() const;

    /**
     * @brief Determine whether the content stored in this is the same as that in other
     * @param other
     * @return
     */
    bool compare(const GByteArray &other) const;

    std::string toHexString(bool uppercase = false) const;

    std::string toString() const override;

public:
    /**
     * @brief Parsing data from a string representing hexadecimal
     *
     * @param hexString
     * @return
     */
    static GByteArray fromHexString(const std::string &hexString);

    /**
     * @brief Compressed data
     * @param data
     * @param dataSize
     * @return
     */
    static std::vector<uint8_t> compress(const uint8_t *data, uint32_t dataSize);

    /**
     * @brief Compressed data
     * @param in
     * @return
     */
    static GByteArray compress(const GByteArray &in);

    /**
     * @brief Determine whether the data is compressed data
     * @param data
     * @return
     */
    static bool isCompressed(const GByteArray &data);

    /**
     * @brief Decompress data
     * @param data
     * @param dataSize
     * @param uncompSize
     * @return
     */
    static std::vector<uint8_t> uncompress(const uint8_t *data, uint32_t dataSize, uint32_t uncompSize);

    /**
     * @brief Decompress data
     * @param in
     * @return
     */
    static GByteArray uncompress(const GByteArray &in);

    /**
     * @brief Encoded data is base64
     * @param data
     * @return
     */
    static std::string base64Encode(const GByteArray &data);

    /**
     * @brief Decoding base64
     * @param codes
     * @return
     */
    static GByteArray base64Decode(const std::string &codes);

    /**
     * @brief Calculate the md5 value of input data
     * @param data
     * @return
     */
    static GByteArray md5Sum(const GByteArray &data);

    /**
     * @brief Calculate the sha1 value of input data
     * @param data
     * @return
     */
    static GByteArray sha1Sum(const GByteArray &data);

    /**
     * @brief Calculate the sha256 value of input data
     * @param data
     * @return
     */
    static GByteArray sha256Sum(const GByteArray &data);

private:
    void resize(uint32_t size);

    uint8_t *ptr();

    void byteOrder(uint8_t *data, uint32_t len) const;

    void copyOnWrite();

    static void align(uint32_t *pos, uint32_t alignment);

private:
    friend class GFile;

    mutable uint32_t mWritePos = 0;
    mutable uint32_t mReadPos = 0;
    ByteOrder mByteOrder = ByteOrder::LittleEndian;

    struct BufferRef
    {
        uint8_t *buffer = nullptr;
        uint32_t size = 0;

        explicit BufferRef(uint32_t size);
        ~BufferRef();

        void resize(uint32_t newSize);
    };
    std::shared_ptr<BufferRef> mBufferRef;
};

template<typename T> using is_supported_gb_stream_t = typename std::enable_if<
        std::is_same<char, T>::value ||
        std::is_same<int8_t, T>::value ||
        std::is_same<uint8_t, T>::value ||
        std::is_same<int16_t, T>::value ||
        std::is_same<uint16_t, T>::value ||
        std::is_same<int32_t, T>::value ||
        std::is_same<uint32_t, T>::value ||
        std::is_same<int64_t, T>::value ||
        std::is_same<uint64_t, T>::value ||
        std::is_same<std::string, T>::value ||
        std::is_same<GString, T>::value ||
        std::is_same<GByteArray, T>::value ||
        std::is_same<float, T>::value ||
        std::is_same<double, T>::value ||
        std::is_same<bool, T>::value ||
        std::is_same<GAny, T>::value
>::type;

template<typename Type, typename = is_supported_gb_stream_t<Type>>
inline GByteArray &operator<<(GByteArray &ba, const Type &in)
{
    ba.write(in);
    return ba;
}

template<typename Type, typename = is_supported_gb_stream_t<Type>>
inline const GByteArray &operator>>(const GByteArray &ba, Type &out)
{
    ba.read(out);
    return ba;
}

template<typename Type, typename = is_supported_gb_stream_t<Type>>
inline GByteArray &operator>>(GByteArray &ba, Type &out)
{
    ba.read(out);
    return ba;
}

inline bool operator==(const GByteArray &lhs, const GByteArray &rhs)
{
    return lhs.compare(rhs);
}

inline bool operator!=(const GByteArray &lhs, const GByteArray &rhs)
{
    return !lhs.compare(rhs);
}

GX_NS_END

#endif //GX_GBYTEARRAY_H
