/*
 * Copyright (c) 2020 Fastly, Kazuho Oku
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "picotls/fusion.h"
#include "../deps/picotest/picotest.h"

static const char *tostr(const void *_p, size_t len)
{
    static char *buf;

    if (buf != NULL)
        free(buf);
    buf = malloc(len * 2 + 1);

    const uint8_t *s = _p;
    char *d = buf;

    for (; len != 0; --len) {
        *d++ = "0123456789abcdef"[*s >> 4];
        *d++ = "0123456789abcdef"[*s & 0xf];
        ++s;
    }
    *d = '\0';

    return buf;
}

int main(int argc, char **argv)
{
    static const uint8_t zero[16384] = {}, one[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    {
        static const uint8_t expected[] = {0x03, 0x88, 0xda, 0xce, 0x60, 0xb6, 0xa3, 0x92, 0xf3, 0x28, 0xc2,
                                           0xb9, 0x71, 0xb2, 0xfe, 0x78, 0x97, 0x3f, 0xbc, 0xa6, 0x54, 0x77,
                                           0xbf, 0x47, 0x85, 0xb0, 0xd5, 0x61, 0xf7, 0xe3, 0xfd, 0x6c};
        ptls_fusion_aesgcm_context_t *ctx = ptls_fusion_aesgcm_create(zero, 5 + 16);
        uint8_t encrypted[sizeof(expected)], decrypted[sizeof(expected) - 16];

        ptls_fusion_aesgcm_encrypt(ctx, encrypted, zero, 16, zero, "hello", 5, NULL, NULL);
        ok(memcmp(expected, encrypted, sizeof(expected)) == 0);

        memset(decrypted, 0x55, sizeof(decrypted));
        ok(ptls_fusion_aesgcm_decrypt(ctx, decrypted, expected, 16, zero, "hello", 5, expected + 16, NULL, NULL));
        ok(memcmp(decrypted, zero, sizeof(decrypted)) == 0);

        ptls_fusion_aesgcm_destroy(ctx);
    }

    { /* test capacity */
        static const uint8_t expected[17] = {0x5b, 0x27, 0x21, 0x5e, 0xd8, 0x1a, 0x70, 0x2e, 0x39,
                                             0x41, 0xc8, 0x05, 0x77, 0xd5, 0x2f, 0xcb, 0x57};
        ptls_fusion_aesgcm_context_t *ctx = ptls_fusion_aesgcm_create(zero, 2);
        uint8_t encrypted[17], decrypted[1] = {0x55};
        ptls_fusion_aesgcm_encrypt(ctx, encrypted, "X", 1, zero, "a", 1, NULL, NULL);
        ok(memcmp(expected, encrypted, 17) == 0);
        ok(ptls_fusion_aesgcm_decrypt(ctx, decrypted, expected, 1, zero, "a", 1, expected + 1, NULL, NULL));
        ok('X' == decrypted[0]);
        ptls_fusion_aesgcm_destroy(ctx);
    }

    {
        ptls_fusion_aesgcm_context_t *aead = ptls_fusion_aesgcm_create(zero, sizeof(zero));
        ptls_fusion_aesecb_context_t *ecb = NULL;

        for (int i = 0; i < 2; ++i) {
            uint8_t encrypted[sizeof(zero) + 16], ecbvec[16], decrypted[sizeof(zero)];
#define DOIT(iv, aad, aadlen, ptlen, expected_tag)                                                                                 \
    do {                                                                                                                           \
        memset(ecbvec, 0, sizeof(ecbvec));                                                                                         \
        ptls_fusion_aesgcm_encrypt(aead, encrypted, zero, ptlen, iv, aad, aadlen, ecb, &ecbvec);                                   \
        ok(strcmp(tostr(encrypted + ptlen, 16), expected_tag) == 0);                                                               \
        if (i == 0) {                                                                                                              \
            ok(memcmp(ecbvec, zero, sizeof(ecbvec)) == 0);                                                                         \
        } else {                                                                                                                   \
            ok(strcmp(tostr(ecbvec, sizeof(ecbvec)), "b6aeaffa752dc08b51639731761aed00") == 0);                                    \
        }                                                                                                                          \
        memset(decrypted, 0x55, sizeof(decrypted));                                                                                \
        ok(ptls_fusion_aesgcm_decrypt(aead, decrypted, encrypted, ptlen, iv, aad, aadlen, encrypted + ptlen, NULL, NULL));         \
        ok(memcmp(decrypted, zero, ptlen) == 0);                                                                                   \
    } while (0)

            DOIT(zero, zero, 13, 17, "1b4e515384e8aa5bb781ee12549a2ccf");
            DOIT(zero, zero, 13, 32, "84030586f55adf8ac3c145913c6fd0f8");
            DOIT(zero, zero, 13, 64, "66165d39739c50c90727e7d49127146b");
            DOIT(zero, zero, 13, 65, "eb3b75e1d4431e1bb67da46f6a1a0edd");
            DOIT(zero, zero, 13, 79, "8f4a96c7390c26bb15b68865e6a861b9");
            DOIT(zero, zero, 13, 80, "5cc2554857b19e7a9e18d015feac61fd");
            DOIT(zero, zero, 13, 81, "5a65f0d4db36c981bf7babd11691fe78");
            DOIT(zero, zero, 13, 95, "6a8a51152efe928999a610d8a7b1df9d");
            DOIT(zero, zero, 13, 96, "6b9c468e24ed96010687f3880a044d42");
            DOIT(zero, zero, 13, 97, "1b4eb785b884a7d4fdebaff81c1c12e8");

            DOIT(zero, zero, 22, 1328, "0507baaece8d573774c94e8103821316");
            DOIT(zero, zero, 21, 1329, "dd70d59030eadb6313e778046540a253");
            DOIT(zero, zero, 20, 1330, "f1b456b955afde7603188af0124a32ef");

            DOIT(zero, zero, 13, 1337, "a22deec51250a7eb1f4384dea5f2e890");
            DOIT(zero, zero, 12, 1338, "42102b0a499b2efa89702ece4b0c5789");
            DOIT(zero, zero, 11, 1339, "9827f0b34252160d0365ffaa9364bedc");

            DOIT(zero, zero, 0, 80, "98885a3a22bd4742fe7b72172193b163");
            DOIT(zero, zero, 0, 96, "afd649fc51e14f3966e4518ad53b9ddc");

#undef DOIT

            ecb = malloc(sizeof(*ecb));
            ptls_fusion_aesecb_init(ecb, one);
        }

        ptls_fusion_aesecb_dispose(ecb);
        free(ecb);
        ptls_fusion_aesgcm_destroy(aead);
    }

    return done_testing();
}
