#ifndef AMA_SOURCE_UNICODE_H
#define AMA_SOURCE_UNICODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define AMA_UNICODE_VERSION "17.0.0"

/* Decode one shortest-form scalar. On failure neither output changes.
 * Validation reports the leading-byte offset of the first malformed sequence. */
bool utf8_decode(const unsigned char *bytes, size_t length,
    uint32_t *scalar, size_t *width);
bool utf8_validate(const unsigned char *bytes, size_t length, size_t *error_offset);
bool unicode_identifier_start(uint32_t scalar);
bool unicode_identifier_continue(uint32_t scalar);
bool unicode_identifier(const unsigned char *bytes, size_t length, size_t *error_offset);

#endif
