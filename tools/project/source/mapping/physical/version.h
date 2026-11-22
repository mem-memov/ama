/** @file
 * @brief Validate explicit ordinary destination-version storage authorities.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * uses a designated destination version, never a guessed owner or first-match
 * duplicate, for additional source, associated test and declared input material.
 */
#ifndef AMA_SOURCE_VERSION_PLACEMENT_H
#define AMA_SOURCE_VERSION_PLACEMENT_H
#include <stdbool.h>
#include <stddef.h>
/** Borrow canonical ordinary version identity and normalized absolute physical
 * directory key. This is a proposal, not a fabricated selected ModuleVersion.
 */
typedef struct { const char *identity, *physical; } SourceVersionPlacement;
/** Accept an empty table, or unique canonical ordinary version identities with
 * absolute physical roots. Complete physical normalization is the caller's
 * prior obligation. Refuse NULL storage with nonzero count, NULL fields,
 * non-version/standard names and repeated identities, even equal repeats.
 * No I/O/allocation/output mutation. O(V squared) exact name comparisons and
 * O(total identity bytes) parsing; constant auxiliary storage.
 */
bool source_version_placements_valid(const SourceVersionPlacement *, size_t);
#endif
