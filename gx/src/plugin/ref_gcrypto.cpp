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

#include <gx/gcrypto.h>


GX_NS_BEGIN

void refGCrypto()
{
    Class<GCrypto>("Gx", "GCrypto", "Crypto algorithm based on ECC.")
            .staticFunc("randomBytes", &GCrypto::randomBytes)
            .staticFunc("signKeyPair", &GCrypto::signKeyPair)
            .staticFunc("sign", &GCrypto::sign)
            .staticFunc("signOpen", &GCrypto::signOpen)
            .staticFunc("boxKeyPair", &GCrypto::boxKeyPair)
            .staticFunc("box", &GCrypto::box)
            .staticFunc("boxOpen", &GCrypto::boxOpen)
            .staticFunc("boxBefore", &GCrypto::boxBefore)
            .staticFunc("boxAfter", &GCrypto::boxAfter)
            .staticFunc("boxOpenAfter", &GCrypto::boxOpenAfter);

    Class<GCrypto::SignKey>("Gx", "SignKey", "GCrypto SignKey.")
            .readOnly("publicKey", &GCrypto::SignKey::publicKey)
            .readOnly("secureKey", &GCrypto::SignKey::secureKey);

    Class<GCrypto::CryptoKey>("Gx", "CryptoKey", "GCrypto CryptoKey.")
            .readOnly("publicKey", &GCrypto::CryptoKey::publicKey)
            .readOnly("secureKey", &GCrypto::CryptoKey::secureKey);
}

GX_NS_END
