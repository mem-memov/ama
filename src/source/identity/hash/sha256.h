/**
 * @file
 * @brief Declare the deterministic SHA-256 candidate digest for identity registries.
 *
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02)
 * The digest selects candidates, never substitutes for complete-key equality.
 */
#ifndef AMA_IDENTITY_SHA256_H
#define AMA_IDENTITY_SHA256_H

#include <stddef.h>
#include <stdint.h>

/* FIPS 180-4 SHA-256, 32 bytes, input shorter than 2^61 bytes. */
/** Compute FIPS 180-4 SHA-256 for a finite input shorter than 2^61 bytes.
 * @param[in] bytes Borrowed input; null permitted only for zero length.
 * @param[out] digest Caller supplies 32 writable bytes not overlapping input. No allocation or collision assumption.
 */
void identity_sha256(const unsigned char *bytes, size_t length, unsigned char digest[32]);

#endif
