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

#include "gx/ghash_sum.h"

#include "algorithm/md5.h"
#include "algorithm/sha1.h"
#include "algorithm/sha256.h"


GX_NS_BEGIN

class Md5HashJob : public GHashJob
{
public:
    explicit Md5HashJob()
    {
        fMD5Init(&mContext);
    }

    ~Md5HashJob() override
    = default;

    void update(const GByteArray &data) override
    {
        fMD5Update(&mContext, data.data(), data.size());
    }

    void update(const uint8_t *data, uint32_t size) override
    {
        fMD5Update(&mContext, data, size);
    }

    GByteArray final() override
    {
        uint8_t digest[MD5_BLOCK_SIZE];
        fMD5Final(&mContext, digest);
        return GByteArray(digest, MD5_BLOCK_SIZE);
    }

private:
    Md5Context mContext{};
};

class Sha1Job : public GHashJob
{
public:
    explicit Sha1Job()
    {
        fSHA1Init(&mContext);
    }

    ~Sha1Job() override
    = default;

    void update(const GByteArray &data) override
    {
        fSHA1Update(&mContext, data.data(), data.size());
    }

    void update(const uint8_t *data, uint32_t size) override
    {
        fSHA1Update(&mContext, data, size);
    }

    GByteArray final() override
    {
        uint8_t digest[SHA1_BLOCK_SIZE];
        fSHA1Final(&mContext, digest);
        return GByteArray(digest, SHA1_BLOCK_SIZE);
    }

private:
    SHA1Context mContext{};
};

class Sha256Job : public GHashJob
{
public:
    explicit Sha256Job()
    {
        fSHA256Init(&mContext);
    }

    ~Sha256Job() override
    = default;

    void update(const GByteArray &data) override
    {
        fSHA256Update(&mContext, data.data(), data.size());
    }

    void update(const uint8_t *data, uint32_t size) override
    {
        fSHA256Update(&mContext, data, size);
    }

    GByteArray final() override
    {
        uint8_t digest[SHA256_BLOCK_SIZE];
        fSHA256Final(&mContext, digest);
        return GByteArray(digest, SHA256_BLOCK_SIZE);
    }

private:
    SHA256Context mContext{};
};


std::unique_ptr<GHashJob> GHashSum::hashSum(GHashSum::HashType hashType)
{
    switch (hashType) {
        case Md5:
            return std::make_unique<Md5HashJob>();
        case Sha1:
            return std::make_unique<Sha1Job>();
        case Sha256:
            return std::make_unique<Sha256Job>();
    }
    return nullptr;
}

GX_NS_END
