/*
 *  Copyright (c) hikyuu
 *  Created on: 2021/12/06
 *      Author: fasiondog
 *
 *  注: boost的md5计算在windows和linux平台上结果不一致，所以改从 dlib 移植
 */

#include "arithmetic.h"
#include "md5.h"
#include "Log.h"

namespace hku {

inline uint32_t F(uint32_t x, uint32_t y, uint32_t z) {
    return ((x & y) | ((~x) & z));
}

// ------------------------------------------------------------------------------------

inline uint32_t G(uint32_t x, uint32_t y, uint32_t z) {
    return ((x & z) | (y & (~z)));
}

// ------------------------------------------------------------------------------------

inline uint32_t H(uint32_t x, uint32_t y, uint32_t z) {
    return (x ^ y ^ z);
}

// ------------------------------------------------------------------------------------

inline uint32_t I(uint32_t x, uint32_t y, uint32_t z) {
    return (y ^ (x | (~z)));
}

// ------------------------------------------------------------------------------------

inline uint32_t rotate_left(uint32_t x, uint32_t n) {
    return ((x << n) | (x >> (32 - n)));
}

// ------------------------------------------------------------------------------------

inline void FF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s,
               uint32_t ac) {
    a += F(b, c, d) + x + ac;
    a = rotate_left(a, s);
    a += b;
}

// ------------------------------------------------------------------------------------

inline void GG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s,
               uint32_t ac) {
    a += G(b, c, d) + x + ac;
    a = rotate_left(a, s);
    a += b;
}

// ------------------------------------------------------------------------------------

inline void HH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s,
               uint32_t ac) {
    a += H(b, c, d) + x + ac;
    a = rotate_left(a, s);
    a += b;
}

// ------------------------------------------------------------------------------------

inline void II(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s,
               uint32_t ac) {
    a += I(b, c, d) + x + ac;
    a = rotate_left(a, s);
    a += b;
}

void scramble_block(uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d, uint32_t* x) {
    const uint32_t S11 = 7;
    const uint32_t S12 = 12;
    const uint32_t S13 = 17;
    const uint32_t S14 = 22;
    const uint32_t S21 = 5;
    const uint32_t S22 = 9;
    const uint32_t S23 = 14;
    const uint32_t S24 = 20;
    const uint32_t S31 = 4;
    const uint32_t S32 = 11;
    const uint32_t S33 = 16;
    const uint32_t S34 = 23;
    const uint32_t S41 = 6;
    const uint32_t S42 = 10;
    const uint32_t S43 = 15;
    const uint32_t S44 = 21;

    // round 1
    FF(a, b, c, d, x[0], S11, 0xd76aa478);   // 1
    FF(d, a, b, c, x[1], S12, 0xe8c7b756);   // 2
    FF(c, d, a, b, x[2], S13, 0x242070db);   // 3
    FF(b, c, d, a, x[3], S14, 0xc1bdceee);   // 4
    FF(a, b, c, d, x[4], S11, 0xf57c0faf);   // 5
    FF(d, a, b, c, x[5], S12, 0x4787c62a);   // 6
    FF(c, d, a, b, x[6], S13, 0xa8304613);   // 7
    FF(b, c, d, a, x[7], S14, 0xfd469501);   // 8
    FF(a, b, c, d, x[8], S11, 0x698098d8);   // 9
    FF(d, a, b, c, x[9], S12, 0x8b44f7af);   // 10
    FF(c, d, a, b, x[10], S13, 0xffff5bb1);  // 11
    FF(b, c, d, a, x[11], S14, 0x895cd7be);  // 12
    FF(a, b, c, d, x[12], S11, 0x6b901122);  // 13
    FF(d, a, b, c, x[13], S12, 0xfd987193);  // 14
    FF(c, d, a, b, x[14], S13, 0xa679438e);  // 15
    FF(b, c, d, a, x[15], S14, 0x49b40821);  // 16

    // Round 2
    GG(a, b, c, d, x[1], S21, 0xf61e2562);   // 17
    GG(d, a, b, c, x[6], S22, 0xc040b340);   // 18
    GG(c, d, a, b, x[11], S23, 0x265e5a51);  // 19
    GG(b, c, d, a, x[0], S24, 0xe9b6c7aa);   // 20
    GG(a, b, c, d, x[5], S21, 0xd62f105d);   // 21
    GG(d, a, b, c, x[10], S22, 0x2441453);   // 22
    GG(c, d, a, b, x[15], S23, 0xd8a1e681);  // 23
    GG(b, c, d, a, x[4], S24, 0xe7d3fbc8);   // 24
    GG(a, b, c, d, x[9], S21, 0x21e1cde6);   // 25
    GG(d, a, b, c, x[14], S22, 0xc33707d6);  // 26
    GG(c, d, a, b, x[3], S23, 0xf4d50d87);   // 27
    GG(b, c, d, a, x[8], S24, 0x455a14ed);   // 28
    GG(a, b, c, d, x[13], S21, 0xa9e3e905);  // 29
    GG(d, a, b, c, x[2], S22, 0xfcefa3f8);   // 30
    GG(c, d, a, b, x[7], S23, 0x676f02d9);   // 31
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);  // 32

    // Round 3
    HH(a, b, c, d, x[5], S31, 0xfffa3942);   // 33
    HH(d, a, b, c, x[8], S32, 0x8771f681);   // 34
    HH(c, d, a, b, x[11], S33, 0x6d9d6122);  // 35
    HH(b, c, d, a, x[14], S34, 0xfde5380c);  // 36
    HH(a, b, c, d, x[1], S31, 0xa4beea44);   // 37
    HH(d, a, b, c, x[4], S32, 0x4bdecfa9);   // 38
    HH(c, d, a, b, x[7], S33, 0xf6bb4b60);   // 39
    HH(b, c, d, a, x[10], S34, 0xbebfbc70);  // 40
    HH(a, b, c, d, x[13], S31, 0x289b7ec6);  // 41
    HH(d, a, b, c, x[0], S32, 0xeaa127fa);   // 42
    HH(c, d, a, b, x[3], S33, 0xd4ef3085);   // 43
    HH(b, c, d, a, x[6], S34, 0x4881d05);    // 44
    HH(a, b, c, d, x[9], S31, 0xd9d4d039);   // 45
    HH(d, a, b, c, x[12], S32, 0xe6db99e5);  // 46
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8);  // 47
    HH(b, c, d, a, x[2], S34, 0xc4ac5665);   // 48

    // Round 4
    II(a, b, c, d, x[0], S41, 0xf4292244);   // 49
    II(d, a, b, c, x[7], S42, 0x432aff97);   // 50
    II(c, d, a, b, x[14], S43, 0xab9423a7);  // 51
    II(b, c, d, a, x[5], S44, 0xfc93a039);   // 52
    II(a, b, c, d, x[12], S41, 0x655b59c3);  // 53
    II(d, a, b, c, x[3], S42, 0x8f0ccc92);   // 54
    II(c, d, a, b, x[10], S43, 0xffeff47d);  // 55
    II(b, c, d, a, x[1], S44, 0x85845dd1);   // 56
    II(a, b, c, d, x[8], S41, 0x6fa87e4f);   // 57
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0);  // 58
    II(c, d, a, b, x[6], S43, 0xa3014314);   // 59
    II(b, c, d, a, x[13], S44, 0x4e0811a1);  // 60
    II(a, b, c, d, x[4], S41, 0xf7537e82);   // 61
    II(d, a, b, c, x[11], S42, 0xbd3af235);  // 62
    II(c, d, a, b, x[2], S43, 0x2ad7d2bb);   // 63
    II(b, c, d, a, x[9], S44, 0xeb86d391);   // 64
}

std::string HKU_UTILS_API md5(const unsigned char* input, size_t len) {
    HKU_CHECK(input, "char *buf is null!");
    // make a temp version of input with enough space for padding and len appended
    unsigned long extra_len = 64 - len % 64;
    if (extra_len <= 8)
        extra_len += 64;
    unsigned char* temp = new unsigned char[extra_len + len];

    // number of 16 word blocks
    const unsigned long N = (extra_len + static_cast<unsigned long>(len)) / 64;

    const unsigned char* input2 = input;
    unsigned char* temp2 = temp;
    unsigned char* end = temp + len;

    // copy input into temp
    while (temp2 != end) {
        *temp2 = *input2;
        ++temp2;
        ++input2;
    }

    // pad temp
    end += extra_len - 8;
    *temp2 = static_cast<unsigned char>(0x80);
    ++temp2;
    while (temp2 != end) {
        *temp2 = 0;
        ++temp2;
    }

    // make len the number of bits in the original message
    // but first multiply len by 8 and since len is only 32 bits the number might
    // overflow so we will carry out the multiplication manually and end up with
    // the result in the base 65536 number with three digits
    // result = low + high*65536 + upper*65536*65536
    unsigned long low = len & 0xFFFF;
    unsigned long high = static_cast<unsigned long>(len) >> 16;
    unsigned long upper;
    unsigned long tmp;
    tmp = low * 8;
    low = tmp & 0xFFFF;
    tmp = high * 8 + (tmp >> 16);
    high = tmp & 0xFFFF;
    upper = tmp >> 16;

    // append the length
    *temp2 = static_cast<unsigned char>(low & 0xFF);
    ++temp2;
    *temp2 = static_cast<unsigned char>((low >> 8) & 0xFF);
    ++temp2;
    *temp2 = static_cast<unsigned char>((high) & 0xFF);
    ++temp2;
    *temp2 = static_cast<unsigned char>((high >> 8) & 0xFF);
    ++temp2;
    *temp2 = static_cast<unsigned char>((upper) & 0xFF);
    ;
    ++temp2;
    *temp2 = static_cast<unsigned char>((upper >> 8) & 0xFF);
    ;
    ++temp2;
    *temp2 = 0;
    ++temp2;
    *temp2 = 0;

    uint32_t a = 0x67452301;
    uint32_t b = 0xefcdab89;
    uint32_t c = 0x98badcfe;
    uint32_t d = 0x10325476;

    // an array of 16 words
    uint32_t x[16];

    for (unsigned long i = 0; i < N; ++i) {
        // copy a block of 16 words from m into x
        for (unsigned long j = 0; j < 16; ++j) {
            x[j] = ((static_cast<uint32_t>(temp[4 * (j + 16 * i) + 3]) << 24) |
                    (static_cast<uint32_t>(temp[4 * (j + 16 * i) + 2]) << 16) |
                    (static_cast<uint32_t>(temp[4 * (j + 16 * i) + 1]) << 8) |
                    (static_cast<uint32_t>(temp[4 * (j + 16 * i)])));
        }

        uint32_t aa = a;
        uint32_t bb = b;
        uint32_t cc = c;
        uint32_t dd = d;

        scramble_block(a, b, c, d, x);

        a = a + aa;
        b = b + bb;
        c = c + cc;
        d = d + dd;
    }

    unsigned char output[16];
    // put a, b, c, and d into output
    output[0] = static_cast<unsigned char>((a) & 0xFF);
    output[1] = static_cast<unsigned char>((a >> 8) & 0xFF);
    output[2] = static_cast<unsigned char>((a >> 16) & 0xFF);
    output[3] = static_cast<unsigned char>((a >> 24) & 0xFF);

    output[4] = static_cast<unsigned char>((b) & 0xFF);
    output[5] = static_cast<unsigned char>((b >> 8) & 0xFF);
    output[6] = static_cast<unsigned char>((b >> 16) & 0xFF);
    output[7] = static_cast<unsigned char>((b >> 24) & 0xFF);

    output[8] = static_cast<unsigned char>((c) & 0xFF);
    output[9] = static_cast<unsigned char>((c >> 8) & 0xFF);
    output[10] = static_cast<unsigned char>((c >> 16) & 0xFF);
    output[11] = static_cast<unsigned char>((c >> 24) & 0xFF);

    output[12] = static_cast<unsigned char>((d) & 0xFF);
    output[13] = static_cast<unsigned char>((d >> 8) & 0xFF);
    output[14] = static_cast<unsigned char>((d >> 16) & 0xFF);
    output[15] = static_cast<unsigned char>((d >> 24) & 0xFF);

    delete[] temp;
    return byteToHexStr((const char*)output, 16);
}

}  // namespace hku