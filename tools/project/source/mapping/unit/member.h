/** @file
 * @brief Place one exact structural member relative to a copied module main.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * retains complete units; [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves physical main/companion/plus geometry without rewriting logical names.
 */
#ifndef AMA_SOURCE_UNIT_MEMBER_PLACEMENT_H
#define AMA_SOURCE_UNIT_MEMBER_PLACEMENT_H
#include "source/memory/arena.h"
#include <stdbool.h>
/** OUTSIDE is exact nonmembership, distinct from malformed interface and memory failure. */
typedef enum { SOURCE_UNIT_PATH_OK, SOURCE_UNIT_PATH_OUTSIDE,
    SOURCE_UNIT_PATH_INVALID, SOURCE_UNIT_PATH_MEMORY } SourceUnitPathStatus;
/** Borrow normalized absolute captured keys during the call. Original/destination
 * mains are regular-file keys (no root or trailing slash). Recognize exactly a
 * regular main, regular flat basename-dot-nonempty companion, directory basename+
 * root or any captured descendant below basename+/. An unrelated/wrong-shape
 * member returns OUTSIDE without allocation. On success own the complete final
 * key in arena, preserving suffix bytes. Other outcomes preserve output and the
 * original keys; failed scratch follows arena lifetime. No I/O, source parsing,
 * logical-name inference or normalization. O(key bytes) time/storage, checked
 * joining arithmetic. Complete units, capture authority, final plus topology,
 * collision/removal/configuration/freshness and publication remain caller proofs.
 */
SourceUnitPathStatus source_unit_member_path(SourceArena *, const char *original_main,
    const char *destination_main, const char *member, bool directory, const char **output);
#endif
