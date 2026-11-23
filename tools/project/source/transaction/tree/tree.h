/** @file
 * @brief Materialize and verify complete private recovery trees with explicit ownership.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires verified prepared copies and saved originals before source removal.
 */
#ifndef AMA_SOURCE_TRANSACTION_TREE_H
#define AMA_SOURCE_TRANSACTION_TREE_H
#include "../forest/forest.h"
/** Callbacks precede explicit preparation/reopen boundaries; cleanup never invokes cancellation. */
typedef enum { SOURCE_TREE_CREATE, SOURCE_TREE_WRITE, SOURCE_TREE_MODE,
    SOURCE_TREE_SYNC, SOURCE_TREE_VERIFY, SOURCE_TREE_OPEN } SourceTreeOperation;
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
/** Exact reopen prepares material; cleanup reopen admits only two recorded
 * cleanup transitions: known entries may already be absent and owned directories
 * may have their recorded mode OR 0700. Outer recovery must prove cleanup authority. */
typedef enum { SOURCE_TREE_ADOPT_EXACT, SOURCE_TREE_ADOPT_CLEANUP } SourceTreeAdoption;
/** Reopen once from trusted recorded operation-owned material. Require actual
 * recorded device/inode/mode classes, not projected synthetic classes. Cleanup
 * admission additionally accepts only the documented missing/mode transitions;
 * it never publishes ready. Exact admission opens only
 * no-follow descriptor-relative components and verify full bytes/membership before
 * ready. No creation, writes or permission changes. A failed partial adoption
 * retains descriptors, but cannot authorize removal of unopened existing children.
 * The outer journal proves the private parent and record ownership; schema alone
 * is never adoption authority. Borrow all record/name/parent owners through teardown.
 */
SourceMaterialTreeStatus source_material_tree_adopt(SourceMaterialTree *, int, const char *,
    const SourceCatalogImageEntry *const *, size_t, SourceTreeAdoption, SourceTreeControl);
/** Verify a ready tree and retain its actual created identities under original
 * material keys for lossless journal emission. Rows belong to the supplied arena;
 * path/content bytes still borrow the immutable forest/record owner. Publish output
 * only after success. Allocation failure retains all recovery material. No writes.
 */
bool source_material_tree_snapshot(SourceMaterialTree *, SourceArena *, const SourceCatalogImageEntry **);
/** Recheck retained identity/mode, exact written byte prefixes and full child sets.
 * No source changes; false retains failed_path/system_error and recovery data. */
bool source_material_tree_verify(SourceMaterialTree *);
/** Match one caller-observed inode against a retained created entry's material
 * key, device/inode and current owned mode. Accept identified failed prefixes;
 * reject uncreated, removed, unknown or destroyed entries. No I/O, allocation
 * or mutation; O(rows * key bytes), constant scratch. This proves recorded
 * ownership only, never complete byte/membership equality or current filesystem
 * truth. The caller must supply its actual no-follow observation and separately
 * verify content/membership. This narrow query lets a private workspace join
 * independently owned request files without exposing internal descriptors or
 * duplicating cleanup under [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
bool source_material_tree_owns(const SourceMaterialTree *, const char *, const struct stat *);
/** Verify all created material before the first unlink, then remove children
 * before parents. Missing already-cleaned entries are supported on a repeated
 * cleanup. Replacements/additional names refuse without deleting them. Failure
 * retains material/descriptors for retry/reporting; no callback can cancel cleanup.
 * Caller must maintain exclusive private-directory access throughout. */
bool source_material_tree_cleanup(SourceMaterialTree *);
#endif
