#ifndef AMA_IDENTITY_SHA256_H
#define AMA_IDENTITY_SHA256_H

#include <stddef.h>
#include <stdint.h>

/* FIPS 180-4 SHA-256, 32 bytes, input shorter than 2^61 bytes. */
void identity_sha256(const unsigned char *bytes, size_t length, unsigned char digest[32]);

#endif
