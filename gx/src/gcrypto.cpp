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

#include "gx/gcrypto.h"

#include <gx/debug.h>

#include <memory>
#include <memory.h>
#include <math.h>

extern "C"
{
#include <tweetnacl/tweetnacl.h>

#if GX_PLATFORM_WINDOWS

#include <windows.h>
#include <wincrypt.h>

#define NCP ((HCRYPTPROV) 0)

HCRYPTPROV sHProvider = NCP;

void randombytes_(unsigned char *x, unsigned long long xlen)
{
    unsigned i;
    BOOL ret;

    if (sHProvider == NCP) {
        for (;;) {
            ret = CryptAcquireContext(&sHProvider, NULL, NULL,
                                      PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT);
            if (ret != FALSE) {
                break;
            }
            Sleep(1);
        }
    }
    while (xlen > 0) {
        if (xlen < 1048576) {
            i = (unsigned) xlen;
        } else {
            i = 1048576;
        }

        ret = CryptGenRandom(sHProvider, i, x);
        if (ret == FALSE) {
            Sleep(1);
            continue;
        }
        x += i;
        xlen -= i;
    }
}

//int randombytes_close(void)
//{
//    int rc = -1;
//    if ((sHProvider != NCP) && (CryptReleaseContext(sHProvider, 0) != FALSE)) {
//        sHProvider = NCP;
//        rc = 0;
//    }
//    return rc;
//}

#else

#include <unistd.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int fd = -1;

void randombytes_(unsigned char *x, unsigned long long xlen)
{
    int i;

    if (fd == -1) {
        for (;;) {
            int flags = O_RDONLY;
//            flags |= O_CLOEXEC;
            fd = open("/dev/urandom", flags);
            if (fd != -1) {
                break;
            }
            sleep(1);
        }
#ifdef FD_CLOEXEC
        int rc = fcntl(fd, F_SETFD, FD_CLOEXEC);
        assert (rc != -1);
#endif
    }

    while (xlen > 0) {
        if (xlen < 1048576) {
            i = xlen;
        } else {
            i = 1048576;
        }

        i = read(fd, x, i);

        if (i < 1) {
            sleep(1);
            continue;
        }
        x += i;
        xlen -= i;
    }
}

//int randombytes_close(void)
//{
//    int rc = -1;
//    if (fd != -1 && close(fd) == 0) {
//        fd = -1;
//        rc = 0;
//    }
//    return rc;
//}

#endif
}


GX_NS_BEGIN

GByteArray GCrypto::randomBytes(int32_t len)
{
    std::vector<uint8_t> data(len);
    randombytes_(&data[0], len);

    GByteArray ret(data.data(), (int32_t) len);
    return ret;
}

GCrypto::SignKey GCrypto::signKeyPair()
{
    unsigned char pubKey[crypto_sign_PUBLICKEYBYTES];
    unsigned char secKey[crypto_sign_SECRETKEYBYTES];

    memset(pubKey, 0, sizeof(pubKey));
    memset(secKey, 0, sizeof(secKey));

    SignKey key;
    if (crypto_sign_keypair(pubKey, secKey) == 0) {
        key.publicKey.write(pubKey, crypto_sign_PUBLICKEYBYTES);
        key.secureKey.write(secKey, crypto_sign_SECRETKEYBYTES);
    }

    return key;
}

GByteArray GCrypto::sign(const GByteArray &data, const GCrypto::CryptoSecKey &secKey)
{
    uint32_t n = data.size();

    unsigned long long smLen = n + 64;

    std::vector<uint8_t> smBuff(smLen);

    if (crypto_sign(smBuff.data(), &smLen, data.data(), n, secKey.data()) == 0) {
        GByteArray sm(smBuff.data(), (int32_t)smLen);
        return sm;
    }

    return GByteArray();
}

GByteArray GCrypto::signOpen(const GByteArray &data, const GCrypto::CryptoPubKey &pubKey)
{
    uint32_t smLen = data.size();

    unsigned long long mLen = std::max((uint64_t)0, (uint64_t)(smLen - 64));
    std::vector<uint8_t> mBuff(smLen);

    if (crypto_sign_open(mBuff.data(), &mLen, data.data(), smLen, pubKey.data()) == 0) {
        GByteArray m(mBuff.data(), (int32_t)mLen);
        return m;
    }

    return GByteArray();
}

GCrypto::CryptoKey GCrypto::boxKeyPair()
{
    unsigned char pubKey[crypto_box_PUBLICKEYBYTES];
    unsigned char secKey[crypto_box_SECRETKEYBYTES];

    memset(pubKey, 0, sizeof(pubKey));
    memset(secKey, 0, sizeof(secKey));

    CryptoKey key;
    if (crypto_box_keypair(pubKey, secKey) == 0) {
        key.publicKey.write(pubKey, crypto_box_PUBLICKEYBYTES);
        key.secureKey.write(secKey, crypto_box_SECRETKEYBYTES);
    }

    return key;
}

GByteArray GCrypto::box(const GByteArray &data, const GByteArray &nonce, const GCrypto::CryptoPubKey &bPubKey,
                        const GCrypto::CryptoSecKey &aSecKey)
{
    GX_ASSERT(nonce.size() <= crypto_box_NONCEBYTES);

    std::vector<uint8_t> dataBuff(crypto_box_ZEROBYTES + data.size());
    std::fill(dataBuff.begin(), dataBuff.begin() + crypto_box_ZEROBYTES, 0);
    memcpy(dataBuff.data() + crypto_box_ZEROBYTES, data.data(), data.size());

    std::vector<uint8_t> nonceData(crypto_box_NONCEBYTES, 0);
    memcpy(nonceData.data(), nonce.data(), nonce.size());

    std::vector<uint8_t> boxBuff(crypto_box_BOXZEROBYTES + 16 + data.size(), 0);

    int rc = crypto_box(boxBuff.data(), dataBuff.data(), dataBuff.size(), nonceData.data(), bPubKey.data(), aSecKey.data());
    if (rc == 0) {
        GByteArray box;
        box.write(boxBuff.data() + crypto_box_BOXZEROBYTES, data.size() + 16); // +16
        return box;
    }

    return GByteArray();
}

GByteArray GCrypto::boxOpen(const GByteArray &data, const GByteArray &nonce, const GCrypto::CryptoPubKey &aPubKey,
                            const GCrypto::CryptoSecKey &bSecKey)
{
    GX_ASSERT(nonce.size() <= crypto_box_NONCEBYTES);

    std::vector<uint8_t> boxBuff(crypto_box_BOXZEROBYTES + data.size(), 0);
    memcpy(boxBuff.data() + crypto_box_BOXZEROBYTES, data.data(), data.size());

    std::vector<uint8_t> nonceData(crypto_box_NONCEBYTES, 0);
    memcpy(&nonceData[0], nonce.data(), nonce.size());

    std::vector<uint8_t> unboxBuff(crypto_box_ZEROBYTES + data.size(), 0);

    int rc = crypto_box_open(unboxBuff.data(), boxBuff.data(), boxBuff.size(), nonceData.data(), aPubKey.data(), bSecKey.data());
    if (rc == 0) {
        GByteArray unbox;
        unbox.write(&unboxBuff[crypto_box_ZEROBYTES], data.size() - 16); // -16
        return unbox;
    }

    return GByteArray();
}

GCrypto::CryptoShareKey GCrypto::boxBefore(const GCrypto::CryptoPubKey &bPubKey,
                                           const GCrypto::CryptoSecKey &aSecKey)
{
    std::vector<uint8_t> sharedKey(32);

    if (crypto_box_beforenm(sharedKey.data(), bPubKey.data(), aSecKey.data()) == 0) {
        GByteArray key(sharedKey.data(), 32);
        return key;
    }

    return GByteArray();
}

GByteArray GCrypto::boxAfter(const GByteArray &data, const GByteArray &nonce, const GCrypto::CryptoShareKey &key)
{
    GX_ASSERT(nonce.size() <= crypto_box_NONCEBYTES);

    std::vector<uint8_t> dataBuff(crypto_box_ZEROBYTES + data.size());
    std::fill(dataBuff.begin(), dataBuff.begin() + crypto_box_ZEROBYTES, 0);
    memcpy(dataBuff.data() + crypto_box_ZEROBYTES, data.data(), data.size());

    std::vector<uint8_t> nonceData(crypto_box_NONCEBYTES, 0);
    memcpy(nonceData.data(), nonce.data(), nonce.size());

    std::vector<uint8_t> boxBuff(crypto_box_BOXZEROBYTES + 16 + data.size(), 0);

    int rc = crypto_box_afternm(boxBuff.data(), dataBuff.data(), dataBuff.size(), nonceData.data(), key.data());
    if (rc == 0) {
        GByteArray box;
        box.write(boxBuff.data() + crypto_box_BOXZEROBYTES, data.size() + 16); // +16
        return box;
    }

    return GByteArray();
}

GByteArray GCrypto::boxOpenAfter(const GByteArray &data, const GByteArray &nonce,
                                 const GCrypto::CryptoShareKey &key)
{
    GX_ASSERT(nonce.size() <= crypto_box_NONCEBYTES);

    std::vector<uint8_t> boxBuff(crypto_box_BOXZEROBYTES + data.size(), 0);
    memcpy(boxBuff.data() + crypto_box_BOXZEROBYTES, data.data(), data.size());

    std::vector<uint8_t> nonceData(crypto_box_NONCEBYTES, 0);
    memcpy(&nonceData[0], nonce.data(), nonce.size());

    std::vector<uint8_t> unboxBuff(crypto_box_ZEROBYTES + data.size(), 0);

    int rc = crypto_box_open_afternm(unboxBuff.data(), boxBuff.data(), boxBuff.size(), nonceData.data(), key.data());
    if (rc == 0) {
        GByteArray unbox;
        unbox.write(&unboxBuff[crypto_box_ZEROBYTES], data.size() - 16); // -16
        return unbox;
    }

    return GByteArray();
}

GX_NS_END
