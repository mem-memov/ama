/**
 * @file
 * @brief Recognize source path spellings independently of normalization and lookup.
 * [SRC-06](../../../spec/language/source-and-names.md#src-06) permits relative
 * and aliased source forms; these are not canonical catalog identities.
 */
#ifndef AMA_SOURCE_PATH_SYNTAX_H
#define AMA_SOURCE_PATH_SYNTAX_H
#include <stdbool.h>
#include <stddef.h>

/** Return the maximal grammatical source path prefix, or zero if none starts.
 * Borrowed bounded bytes are never changed/retained; NULL yields zero. Internal
 * spaces belong to maximal names, never optional separators. Trailing/empty
 * slash components fail the entire prefix. Callers must reject unconsumed
 * suffixes using their surrounding grammar, never retry a shorter path.
 * [SRC-01](../../../spec/language/source-and-names.md#src-01) separates / and //;
 * [SRC-06](../../../spec/language/source-and-names.md#src-06) adds ./, ../ and ~.
 * Allocation-free O(length) time, O(1) auxiliary space; no filesystem effects.
 */
size_t module_path_prefix(const unsigned char *bytes, size_t length);

/** Validate the entire source form. This is syntax only: dot components may
 * require normalization and an absolute endpoint may lack package structure.
 * [SRC-06](../../../spec/language/source-and-names.md#src-06) owns normalization;
 * [SRC-01](../../../spec/language/source-and-names.md#src-01) owns canonical lookup.
 * The bounded/ownership/complexity contract is identical to module_path_prefix.
 */
bool module_path_syntax(const unsigned char *bytes, size_t length);
#endif
