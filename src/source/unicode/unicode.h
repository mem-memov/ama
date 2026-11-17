/**
 * @file
 * @brief Declare strict scalar decoding and pinned Unicode word/complete-name recognition.
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
 * @param[in] bytes Borrowed bounded original UTF-8 bytes; null/empty input cannot decode a scalar.
 * @param[in] length Available byte count; decoding never reads past it.
 * Component contract: [SRC-03](../../../spec/language/source-and-names.md#src-03). */
bool utf8_decode(const unsigned char *bytes, size_t length,
    uint32_t *scalar, size_t *width);
/** Validate a finite byte view, permitting null only at zero length.
 * @param[out] error_offset Set to the lead byte of the first invalid sequence only on false.
 * @param[in] bytes Borrowed original byte view, null only for an empty view.
 * @param[in] length Exact byte length; validation consumes every scalar without normalization.
 * Component contract: [SRC-03](../../../spec/language/source-and-names.md#src-03). */
bool utf8_validate(const unsigned char *bytes, size_t length, size_t *error_offset);
/** Test pinned XID_Start plus underscore; out-of-domain values do not match.
 * Component contract: [SRC-03](../../../spec/language/source-and-names.md#src-03). */
bool unicode_identifier_start(uint32_t scalar);
/** Test pinned XID_Continue plus underscore; original scalar spelling remains unchanged.
 * Component contract: [SRC-03](../../../spec/language/source-and-names.md#src-03). */
bool unicode_identifier_continue(uint32_t scalar);
/** Return the maximal complete Ama word prefix without allocating or changing bytes.
 * A word begins with pinned XID_Start/underscore and continues with pinned
 * XID_Continue/underscore or a hyphen immediately followed by such a continuation.
 * Returning zero means no word starts here. Malformed UTF-8, punctuation or an
 * invalid hyphen ends the prefix before that byte; callers validate their full
 * span or diagnose the unconsumed suffix, never repair it.
 * @param[in] bytes Borrowed bounded original bytes; NULL yields zero.
 * @param[in] length Maximum readable byte count; no terminator is required.
 * @return Consumed byte count in [0,length], including only complete scalars.
 * Component contract: [SRC-03](../../../spec/language/source-and-names.md#src-03)
 * supplies the same word profile to name validation and token recognition. */
size_t unicode_word_prefix(const unsigned char *bytes, size_t length);
/** Recognize one complete Ama word, including isolated internal ASCII hyphens.
 * @param[out] error_offset Set on false to the first unconsumed byte of the
 * maximal word prefix (zero for empty/invalid start), including an invalid hyphen.
 * @param[in] bytes Borrowed nonempty UTF-8 spelling of one identifier word.
 * @param[in] length Exact byte length of that word; spaces belong to a separate grammar layer.
 * Component contract: [SRC-03](../../../spec/language/source-and-names.md#src-03). */
bool unicode_identifier(const unsigned char *bytes, size_t length, size_t *error_offset);

/** Validate one complete name, with one nonempty XID word per single ASCII-space separator.
 * No leading/trailing/repeated space, other whitespace, normalization or case folding.
 * This predicate receives an already delimited span; it does not select tokens.
 * @param[in] bytes Borrowed exact UTF-8 name bytes; never changed or retained.
 * @param[in] length Complete span length, including its internal single spaces.
 * @param[out] error_offset Valid writable storage; changes only on false to the
 * first invalid word/scalar/space byte. Empty input reports zero, trailing space
 * reports that space, and malformed UTF-8 reports its leading byte.
 * Component contract: [SRC-03](../../../spec/language/source-and-names.md#src-03). */
bool unicode_name(const unsigned char *bytes, size_t length, size_t *error_offset);

#endif
