/** @file
 * @brief Prepare one destination's exact preserved-plus-authorized permission set.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * treats shared source/destination configurations as unchanged destination
 * environments; only explicitly copied configurations map existing identities.
 */
#ifndef AMA_SOURCE_COPY_CONFIGURATION_PREPARE_H
#define AMA_SOURCE_COPY_CONFIGURATION_PREPARE_H
#include "../selectors/additions.h"
#include "../selectors/reexpress.h"
/** Keep child failures typed and tied to their actual preparation stage. */
typedef enum { SOURCE_CONFIGURATION_OK, SOURCE_CONFIGURATION_INVALID,
    SOURCE_CONFIGURATION_ROOTS, SOURCE_CONFIGURATION_PERMISSIONS,
    SOURCE_CONFIGURATION_MEMORY, SOURCE_CONFIGURATION_LIMIT,
    SOURCE_CONFIGURATION_EDITS } SourceConfigurationStatus;
/** Borrow child diagnostics and their inputs through caller teardown. */
typedef struct {
    SourceSelectionRootsStatus roots_status;
    SourceSelectionRootsIssue roots;
    SourcePermissionStatus permission_status;
    SourcePermissionIssue permission;
    SourceEditsStatus edits_status;
} SourceConfigurationIssue;
/** Require actual original/final package scopes from valid immutable environment
 * owners and edits over the exact original scope document. Original environment
 * must belong to copy's captured catalog. For an existing destination, including
 * a shared source/destination package.ini, copied=false retains original roots
 * requires the same original/final version identity and forbids version pairs. Only an explicitly copied whole-version config may
 * use copied=true and the outer operation's actual version placement pairs.
 *
 * Derive baseline, then concrete additions against that fixed baseline, then
 * their canonical exact union; prepare selector edits once against original
 * spans. Alias edits for explicitly copied configurations may share this owner
 * but are a separate operation; never rebase aliases in an unchanged shared
 * configuration. Output changes only on success. Arena owns scratch/output;
 * failed edit prefixes remain edit-owned and must be discarded on refusal.
 * No I/O or source mutation. Work is bounded by child finite graph/domain scans
 * plus O(V*(B+A)) union comparisons and O(V) storage. Final independent reparse,
 * exact selection, compiler/graph checks and atomic publication remain required.
 */
SourceConfigurationStatus source_copy_configuration_prepare(SourceArena *, SourceTextEdits *,
    const SourceCopyClosure *, const SourceCloneMap *,
    const SourcePackageEnvironment *original, const SourcePackageScope *original_scope,
    const SourcePackageEnvironment *final, const SourcePackageScope *final_scope,
    bool copied, const SourceSelectionVersionPair *, size_t,
    SourceSelectionRoots *, SourceConfigurationIssue *);
#endif
