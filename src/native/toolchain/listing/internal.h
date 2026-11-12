/**
 * @file
 * @brief Separate bounded symbol-record recognition from exact entry-family matching.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * Borrow finite byte views and retain no listing storage or compiler context.
 */
#ifndef AMA_NATIVE_LISTING_INTERNAL_H
#define AMA_NATIVE_LISTING_INTERNAL_H

#include <stdbool.h>
#include "../symbols.h"

/** Recognize one bounded two-to-four-token supported listing record with a one-byte type and optional hexadecimal fields.
 * @param[out] symbol Published only on success and borrows line storage; blank/malformed records fail.
 * @param[in] line Borrowed complete supported nm record bytes; no terminator is required.
 */
bool ama_listing_record(AmaByteView line, AmaByteView *symbol);
/** Compare a nonempty validated generated entry with an exact symbol or its dot-delimited compiler specialization suffix; no substring/name heuristic.
 */
bool ama_listing_matches(AmaByteView symbol, AmaByteView entry);

#endif
