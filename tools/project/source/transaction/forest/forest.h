/** @file
 * @brief Partition an exact before/after image delta into disjoint recoverable subtree mutations.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * stages complete replacements and retains originals before destructive publication.
 */
#ifndef AMA_SOURCE_TRANSACTION_FOREST_H
#define AMA_SOURCE_TRANSACTION_FOREST_H
#include "../../image/image.h"
#include "source/memory/arena.h"
/** One disjoint mutation root and its complete sorted original/final material.
 * Rows/paths borrow immutable image owners; parent spelling and arrays are arena
 * owned. parent_entry is a complete unchanged original directory witness. */
typedef struct {
    const char *path, *parent, *name;
    const SourceCatalogImageEntry *parent_entry, *before, *after;
    const SourceCatalogImageEntry *const *before_entries, *const *after_entries;
    size_t before_count, after_count;
} SourceMutationRoot;
/** Exact minimal ancestor roots; count zero is a valid no-op. */
typedef struct { const SourceMutationRoot *roots; size_t count; } SourceMutationForest;
/** Reject unavailable material and parent authority instead of guessing absence. */
typedef enum { SOURCE_FOREST_OK, SOURCE_FOREST_INVALID, SOURCE_FOREST_MEMORY,
    SOURCE_FOREST_LIMIT, SOURCE_FOREST_UNOBSERVED, SOURCE_FOREST_UNSUPPORTED,
    SOURCE_FOREST_PARENT } SourceForestStatus;
/** Borrow the first exact missing/unsupported material or parent witness. */
typedef struct { const char *path; const SourceCatalogImageEntry *entry; } SourceForestIssue;
/** Compare exact key sets, known kind/mode/inode class and captured content bytes;
 * timestamps/unused stat fields do not create writes. Unknown equal observations
 * stay untouched. Select changed paths with no changed proper ancestor: they are
 * unique disjoint roots covering every changed row. Retain all material below
 * each root, including unchanged children, so replacement and rollback have a
 * complete tree rather than a partial file merge.
 *
 * Require the same ordered physical observation boundaries in both
 * images; a registry root-setting change alone is never a physical deletion.
 * Require known regular bytes or complete directories throughout each original/
 * final subtree, plus a known complete unchanged original parent for each root.
 * Root-setting compositions must first supply their stable physical parent
 * observations; no host fallback or invented parent root is used here. No I/O,
 * source mutation or semantic/removal authority. Publish output only on success;
 * failed scratch remains arena-owned. Valid image/byte/path owners stay immutable
 * through consumption. Finite scans terminate; O(B+(N+M)² + bytes) comparisons
 * for B physical boundaries, plus
 * sorting, O(N+M + roots*parent bytes) storage for N original and M final rows.
 */
SourceForestStatus source_mutation_forest(SourceArena *, const SourceCatalogImage *, const SourceCatalogImage *,
    SourceMutationForest *, SourceForestIssue *);
#endif
