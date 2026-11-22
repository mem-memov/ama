/** @file
 * @brief Check original surviving consumers before deletion, movement or replacement.
 * [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * forbids removing definitions or declared inputs required by survivors;
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * keeps this pure original-graph predicate outside compiler and mutation state.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains all
 * complete/failed report metadata in the caller's operation arena.
 */
#ifndef AMA_SOURCE_REMOVAL_CHECK_H
#define AMA_SOURCE_REMOVAL_CHECK_H
#include "delivery/source/records.h"
#include "source/memory/arena.h"
/** Original explicit non-module input use. owner is an exact member of the
 * complete original definition export; path is a common-resolver physical key.
 * Original source owners outlive the report under
 * [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03).
 */
typedef struct SourceRemovalInputUse {
    struct SourceRemovalInputUse *next;
    const DeliverySourceDefinition *owner;
    const char *path;
    SourceSpan origin;
} SourceRemovalInputUse;
/** Module and input blockers remain distinct original facts, never predictions
 * of a final replacement graph under
 * [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03).
 */
typedef enum { SOURCE_REMOVAL_MODULE, SOURCE_REMOVAL_INPUT } SourceRemovalBlockKind;
/** One original occurrence. Strings, source spans and owner borrow original
 * snapshot/removal-input owners; only this list node belongs to the result arena.
 * [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * requires reporting outside consumers even when unused or non-executable.
 */
typedef struct SourceRemovalBlocker {
    struct SourceRemovalBlocker *next;
    SourceRemovalBlockKind kind;
    const DeliverySourceDefinition *owner;
    const char *target;
    SourceSpan origin;
} SourceRemovalBlocker;
/** BLOCKED publishes a complete report; allocation/invalid-input failure never
 * publishes a partial report under
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
typedef enum { SOURCE_REMOVAL_OK, SOURCE_REMOVAL_BLOCKED, SOURCE_REMOVAL_INVALID,
    SOURCE_REMOVAL_OUT_OF_MEMORY } SourceRemovalStatus;
/** Report each original surviving definition edge/input use crossing into the
 * exact removed sets. Borrow complete checked source exports and exact input-use
 * metadata; normalized names/paths and all their owners outlive report consumption.
 * Removed definitions may include opaque drafts absent from active exports.
 * Duplicate removed-set entries are harmless. Original use order is preserved;
 * no replacement, clone or runtime reachability can alter this predicate.
 * Output changes only for OK/BLOCKED (NULL for OK). Failure retains owned scratch
 * and leaves output unchanged. No I/O, commands, compiler callbacks or mutation.
 * Complexity is O((E+I)R + IF + NI) string/identity comparisons for N definitions,
 * E edges, I declared uses, R removed identities and F removed input paths;
 * storage is O(reported occurrences). [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * makes this the shared necessary graph/input check; topology, explicit client
 * roots and final/transaction validation remain separate composition obligations.
 */
SourceRemovalStatus source_removal_check(SourceArena *, const DeliverySourceDefinition *,
    const SourceRemovalInputUse *, const char *const *removed, size_t removed_count,
    const char *const *paths, size_t path_count, const SourceRemovalBlocker **output);
#endif
