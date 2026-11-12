/**
 * @file
 * @brief Declare strict scalar decoding and pinned Unicode XID word recognition.
 *
 * [SRC-03](../../../spec/language/source-and-names.md#src-03)
 * Recognize individual words without normalizing or composing complete multiword names.
 */
#ifndef AMA_SOURCE_UNICODE_H
#define AMA_SOURCE_UNICODE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define AMA_UNICODE_VERSION "17.0.0"

/* Decode one shortest-form scalar. On failure neither output changes.
 * Validation reports the leading-byte offset of the first malformed sequence. */
/** Decode one shortest-form Unicode scalar; no normalization or locale lookup.
 * @param[out] scalar Changes only on true.
 * @param[out] width Positive consumed byte count only on true.
 * @return false for null/empty/truncated/malformed bytes without changing either output.
 */
bool utf8_decode(const unsigned char *bytes, size_t length,
    uint32_t *scalar, size_t *width);
/** Validate a finite byte view, permitting null only at zero length.
 * @param[out] error_offset Set to the lead byte of the first invalid sequence only on false.
 */
bool utf8_validate(const unsigned char *bytes, size_t length, size_t *error_offset);
/** Test pinned XID_Start plus underscore; out-of-domain values do not match.
 */
bool unicode_identifier_start(uint32_t scalar);
/** Test pinned XID_Continue plus underscore; original scalar spelling remains unchanged.
 */
bool unicode_identifier_continue(uint32_t scalar);
/** Recognize one nonempty XID word, separately from complete multiword-name grammar.
 * @param[out] error_offset Set to the first failing scalar lead byte on false, including empty input.
 */
bool unicode_identifier(const unsigned char *bytes, size_t length, size_t *error_offset);

#endif
