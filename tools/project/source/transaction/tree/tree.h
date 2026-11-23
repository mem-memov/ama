/** @file
 * @brief Materialize and verify complete private recovery trees with explicit ownership.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires verified prepared copies and saved originals before source removal.
 */
#ifndef AMA_SOURCE_TRANSACTION_TREE_H
#define AMA_SOURCE_TRANSACTION_TREE_H
#include "../forest/forest.h"
/** Each callback precedes one host operation; cleanup never invokes cancellation. */
typedef enum { SOURCE_TREE_CREATE, SOURCE_TREE_WRITE, SOURCE_TREE_MODE,
    SOURCE_TREE_SYNC, SOURCE_TREE_VERIFY } SourceTreeOperation;
/** Borrow cancellation/fault context only through synchronous preparation. */
typedef struct { void *context; bool (*permit)(void *, SourceTreeOperation, const char *); } SourceTreeControl;
/** Preserve refusal, allocation, cancellation, I/O and changed ownership separately. */
typedef enum { SOURCE_MATERIAL_TREE_OK, SOURCE_MATERIAL_TREE_INVALID,
    SOURCE_MATERIAL_TREE_MEMORY, SOURCE_MATERIAL_TREE_CANCELLED,
    SOURCE_MATERIAL_TREE_IO, SOURCE_MATERIAL_TREE_CHANGED } SourceMaterialTreeStatus;
struct SourceMaterialRow;
/** Own descriptors and metadata; borrow immutable complete forest material and
 * an exclusively caller-owned parent descriptor/name until cleanup/destruction.
 * Never copy the owner. Failed prefixes remain owned, never become ready. */
typedef struct {
    SourceArena arena;
    struct SourceMaterialRow *rows;
    size_t count;
    int parent;
    const char *name, *failed_path;
    SourceTreeControl control;
    SourceMaterialTreeStatus status;
    int system_error;
    bool attempted, ready;
} SourceMaterialTree;
/** Initialize without opening files or changing source paths. */
void source_material_tree_init(SourceMaterialTree *, SourceAllocator);
/** Close retained descriptors exactly once, returning the first close errno.
 * Never implicitly remove retained recovery material, including failed prefixes. */
int source_material_tree_destroy(SourceMaterialTree *);
/** Build once under a borrowed private parent using an exclusive single-component
 * name and one complete nonempty before/after forest material array. Prevalidate
 * every relative component and exact parent before creation. Retain descriptors,
 * write exact bytes, apply modes, synchronize and verify complete readback.
 * No original source paths are opened or changed. On failure, explicit cleanup
 * may remove only this owner's unchanged created prefix; unknown material refuses.
 * Caller keeps all expected row/text storage and parent alive. Finite row scans
 * and bytes bound work; O(N² + bytes), O(N) owned descriptors/metadata. No journal
 * or semantic/freshness/publication authority is implied by ready. */
SourceMaterialTreeStatus source_material_tree_prepare(SourceMaterialTree *, int, const char *,
    const SourceCatalogImageEntry *const *, size_t, SourceTreeControl);
/** Recheck retained identity/mode, exact written byte prefixes and full child sets.
 * No source changes; false retains failed_path/system_error and recovery data. */
bool source_material_tree_verify(SourceMaterialTree *);
/** Verify all created material before the first unlink, then remove children
 * before parents. Missing already-cleaned entries are supported on a repeated
 * cleanup. Replacements/additional names refuse without deleting them. Failure
 * retains material/descriptors for retry/reporting; no callback can cancel cleanup.
 * Caller must maintain exclusive private-directory access throughout. */
bool source_material_tree_cleanup(SourceMaterialTree *);
#endif
