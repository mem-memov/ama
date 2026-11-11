#include <string.h>
#include "sha256.h"

static const uint32_t round_constants[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static uint32_t rotate(uint32_t value, unsigned int count) {
    return (value >> count) | (value << (32 - count));
}

static void expand_words(const unsigned char block[64], uint32_t words[64]) {
    for (size_t index = 0; index < 16; index++) {
        size_t offset = index * 4;
        words[index] = ((uint32_t)block[offset] << 24) | ((uint32_t)block[offset + 1] << 16)
            | ((uint32_t)block[offset + 2] << 8) | block[offset + 3];
    }
    for (size_t index = 16; index < 64; index++) {
        uint32_t earlier = words[index - 15];
        uint32_t later = words[index - 2];
        uint32_t small_zero = rotate(earlier, 7) ^ rotate(earlier, 18) ^ (earlier >> 3);
        uint32_t small_one = rotate(later, 17) ^ rotate(later, 19) ^ (later >> 10);
        words[index] = words[index - 16] + small_zero + words[index - 7] + small_one;
    }
}

static void compress_block(uint32_t state[8], const unsigned char block[64]) {
    uint32_t words[64];
    uint32_t working[8];
    expand_words(block, words);
    memcpy(working, state, sizeof(working));
    for (size_t index = 0; index < 64; index++) {
        uint32_t upper_one = rotate(working[4], 6) ^ rotate(working[4], 11) ^ rotate(working[4], 25);
        uint32_t choice = (working[4] & working[5]) ^ (~working[4] & working[6]);
        uint32_t first = working[7] + upper_one + choice + round_constants[index] + words[index];
        uint32_t upper_zero = rotate(working[0], 2) ^ rotate(working[0], 13) ^ rotate(working[0], 22);
        uint32_t majority = (working[0] & working[1]) ^ (working[0] & working[2]) ^ (working[1] & working[2]);
        uint32_t fourth = working[3];
        memmove(working + 1, working, 7 * sizeof(uint32_t));
        working[4] = fourth + first;
        working[0] = first + upper_zero + majority;
    }
    for (size_t index = 0; index < 8; index++) {
        state[index] += working[index];
    }
}

void identity_sha256(const unsigned char *bytes, size_t length, unsigned char digest[32]) {
    uint32_t state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    size_t remaining = length;
    while (remaining >= 64) {
        compress_block(state, bytes);
        bytes += 64;
        remaining -= 64;
    }
    unsigned char padding[128] = {0};
    if (remaining != 0) {
        memcpy(padding, bytes, remaining);
    }
    padding[remaining] = 0x80;
    size_t padded = remaining < 56 ? 64 : 128;
    uint64_t bit_length = (uint64_t)length * 8;
    for (size_t index = 0; index < 8; index++) {
        padding[padded - 1 - index] = (unsigned char)(bit_length >> (index * 8));
    }
    compress_block(state, padding);
    if (padded == 128) {
        compress_block(state, padding + 64);
    }
    for (size_t index = 0; index < 32; index++) {
        digest[index] = (unsigned char)(state[index / 4] >> ((3 - index % 4) * 8));
    }
}
