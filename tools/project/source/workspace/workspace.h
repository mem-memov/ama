/** @file
 * @brief Own private immutable compiler requests outside protected source/destination trees.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * separates preparation effects from compilation; [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires exact input bytes and safe retained ownership after every failed prefix.
 */
#ifndef AMA_SOURCE_WORKSPACE_H
#define AMA_SOURCE_WORKSPACE_H
#include <limits.h>
#include <stdint.h>
#include "../transaction/tree/tree.h"
#include "source/path/physical/physical.h"
#include "delivery/source/snapshot/snapshot.h"
/** The two immutable requests are written at different preparation stages. */
typedef enum { SOURCE_WORKSPACE_ORIGINAL, SOURCE_WORKSPACE_FINAL, SOURCE_WORKSPACE_FILE_COUNT } SourceWorkspaceSlot;
/** Protocol bytes/expected entries remain owned through tree cleanup/destruction. */
typedef struct {
    const char *name;
    SourceCatalogImageEntry entry;
    const SourceCatalogImageEntry *entries[1];
    SourceMaterialTree tree;
    bool attempted;
} SourceWorkspaceFile;
/** Preserve primary and cleanup failures independently. */
typedef enum { SOURCE_WORKSPACE_OK, SOURCE_WORKSPACE_INVALID, SOURCE_WORKSPACE_MEMORY,
    SOURCE_WORKSPACE_LIMIT, SOURCE_WORKSPACE_PHYSICAL, SOURCE_WORKSPACE_BOUNDARY,
    SOURCE_WORKSPACE_IO, SOURCE_WORKSPACE_TREE, SOURCE_WORKSPACE_CHANGED,
    SOURCE_WORKSPACE_SNAPSHOT } SourceWorkspaceStatus;
/** A borrowed exact path and observed child/host cause; no diagnostic inference. */
typedef struct { const char *path; int system_error; SourceMaterialTreeStatus tree_status; } SourceWorkspaceIssue;
/** Binary digit counts bound decimal PID/counter digits; this is not a path cap. */
#define SOURCE_WORKSPACE_NAME_CAPACITY (sizeof(".ama-source--") + sizeof(uintmax_t) * CHAR_BIT + sizeof(size_t) * CHAR_BIT + 1)
/** Noncopyable owner. Borrow control through all writes; own directory descriptors,
 * bytes and shared private-tree owners. ready means the joined private prefix is
 * usable, not language/operation/freshness authority. Explicit cleanup precedes
 * destruction; inspect retained paths before their arena expires. */
typedef struct {
    SourceArena arena;
    SourceManager texts;
    SourceMaterialTree root;
    SourceWorkspaceFile files[SOURCE_WORKSPACE_FILE_COUNT];
    SourceCatalogImageEntry root_entry;
    const SourceCatalogImageEntry *root_entries[1];
    SourcePhysicalPath parent_path;
    SourcePhysicalIssue physical_issue;
    char name[SOURCE_WORKSPACE_NAME_CAPACITY];
    char *path;
    int parent, directory;
    SourceTreeControl control;
    SourceStatus text_status;
    SourceWorkspaceStatus status, cleanup_status;
    SourceWorkspaceIssue issue, cleanup_issue;
    bool attempted, ready, cleaned;
} SourceWorkspace;
/** Initialize every child without I/O. */
void source_workspace_init(SourceWorkspace *, SourceAllocator);
/** Close children/descriptors once and release memory; never delete retained
 * files implicitly. Return the first close errno. Parent/name owners expire last. */
int source_workspace_destroy(SourceWorkspace *);
/** Open once below an explicit existing temporary parent. protected_paths are
 * canonical absolute physical subtree keys prepared by the caller, including
 * semantic roots and complete selected/target trees; borrow only for this call.
 * Refuse a parent inside protection before creation. Choose an unused exact
 * PID/counter name disjoint in both ancestry directions from protection before
 * mkdir. Skip observed existing names without following/deleting them; a race
 * after absence refuses safely. Retain verified shared-tree creation ownership.
 * No source/native/test/compiler/CMake/network execution. Under stable finite
 * parent membership, at most N+P+1 candidates are needed for N existing names and
 * P protected prefixes; checked counter/name exhaustion refuses. O((N+P)*P)
 * scope comparisons plus exact probes/spelling work, and O(parent/name bytes)
 * additional storage beyond shared physical traversal/private-tree bounds.
 * No root authority is inferred from this parent.
 */
SourceWorkspaceStatus source_workspace_open(SourceWorkspace *, const char *parent,
    const char *const *protected_paths, size_t protected_count, SourceTreeControl);
/** Serialize an admitted immutable view once to one slot, then use the existing
 * private-tree writer/readback. Borrow view only synchronously; own resulting
 * bytes. Root cached whole-tree readiness is invalidated as children are added;
 * the workspace checks their joined ownership/membership separately. No compiler
 * invocation or source writes. Encoding, allocation, host and tree failures keep
 * the failed private prefix reportable; repeats refuse without replacing data.
 * Shared encoding/tree bounds plus O(encoded bytes) owned storage apply.
 */
SourceWorkspaceStatus source_workspace_snapshot(SourceWorkspace *, SourceWorkspaceSlot, const DeliverySourceSnapshotView *);
/** Verify current root identity, every created request byte prefix and exact
 * root membership before use. No source mutation; failure invalidates ready and
 * retains the first primary issue. O(file bytes), with exactly two slot owners. */
SourceWorkspaceStatus source_workspace_verify(SourceWorkspace *);
/** Explicitly verify joined ownership, then clean file prefixes and empty root
 * through shared tree cleanup. Preserve foreign/replaced/unknown material and
 * primary failure; report cleanup separately. Repeated successful cleanup is
 * harmless. No recursive pathname deletion or cancellation callback. On refusal,
 * path remains reportable until destruction; no source input is changed. */
SourceWorkspaceStatus source_workspace_cleanup(SourceWorkspace *);
#endif
