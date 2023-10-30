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

#ifndef GX_CRYPTO_H
#define GX_CRYPTO_H

#include "gx/gglobal.h"
#include "gx/base.h"

#include "gbytearray.h"


GX_NS_BEGIN

/**
 * @class GCrypto
 * @brief Provided some algorithms based on ECC encryption
 */
class GX_API GCrypto
{
public:
    constexpr static int32_t NONCE_BYTES = 24;
    constexpr static int32_t SIGN_SECURE_KEY_LEN = 64;
    constexpr static int32_t SIGN_PUBLIC_KEY_LEN = 32;
    constexpr static int32_t CRYPTO_SECURE_KEY_LEN = 64;
    constexpr static int32_t CRYPTO_PUBLIC_KEY_LEN = 64;

    using CryptoPubKey = GByteArray;
    using CryptoSecKey = GByteArray;
    using CryptoShareKey = GByteArray;

    struct SignKey
    {
        CryptoPubKey publicKey;
        CryptoSecKey secureKey;
    };

    struct CryptoKey
    {
        CryptoPubKey publicKey;
        CryptoSecKey secureKey;
    };

public:
    /**
     * @brief Generate a random sequence of len length
     * @param len
     * @return
     */
    static GByteArray randomBytes(int32_t len);

    /**
     * @brief Generate signature key
     * @return
     */
    static SignKey signKeyPair();

    /**
     * @brief signature
     * @param data
     * @param secKey
     * @return
     */
    static GByteArray sign(const GByteArray &data, const CryptoSecKey &secKey);

    /**
     * @brief Unsignature
     * @param data
     * @param pubKey
     * @return
     */
    static GByteArray signOpen(const GByteArray &data, const CryptoPubKey &pubKey);

    /**
     * @brief Generate key for boxing encryption
     * @return
     */
    static CryptoKey boxKeyPair();

    /**
     * @brief Encryption (boxing)
     * @param data      Source data
     * @param nonce     Temporary password, maximum length NONCE_BYTES
     * @param bPubKey   Public key of B-end (receiving)
     * @param aSecKey   Private key of A-end (sender)
     * @return  装箱后的数据
     */
    static GByteArray box(const GByteArray &data, const GByteArray &nonce,
                          const CryptoPubKey &bPubKey, const CryptoSecKey &aSecKey);

    /**
     * @brief Decryption (unboxing)
     * @param data      Encrypted data
     * @param nonce     Temporary password, maximum length NONCE_BYTES
     * @param aPubKey   Public key of A-end (sender)
     * @param bSecKey   Private key of B-end (receiving)
     * @return
     */
    static GByteArray boxOpen(const GByteArray &data, const GByteArray &nonce,
                              const CryptoPubKey &aPubKey, const CryptoSecKey &bSecKey);

    /**
     * @brief Shared Key Construction for Before after Workflow
     * @param bPubKey
     * @param aSecKey
     * @return
     */
    static CryptoShareKey boxBefore(const CryptoPubKey &bPubKey, const CryptoSecKey &aSecKey);

    /**
     * @brief Encryption of Before after workflow
     * @param data  Source data
     * @param nonce Temporary password, maximum length NONCE_BYTES
     * @param key   Shared key generated through boxBefore
     * @return
     */
    static GByteArray boxAfter(const GByteArray &data, const GByteArray &nonce, const CryptoShareKey &key);

    /**
     * @brief Decryption of Before after workflow
     * @param data  Encrypted data
     * @param nonce Temporary password, maximum length NONCE_BYTES
     * @param key   Shared key generated through boxBefore
     * @return
     */
    static GByteArray boxOpenAfter(const GByteArray &data, const GByteArray &nonce, const CryptoShareKey &key);
};

GX_NS_END

#endif //GX_CRYPTO_H
