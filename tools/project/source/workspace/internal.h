/** @file
 * @brief Separate private workspace naming, identity, serialization and cleanup responsibilities.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires failed ownership and safe cleanup to remain distinct from source authority.
 */
#ifndef AMA_SOURCE_WORKSPACE_INTERNAL_H
#define AMA_SOURCE_WORKSPACE_INTERNAL_H
#include "workspace.h"
/** Retain the first primary failure, invalidate readiness, and preserve child evidence. */
SourceWorkspaceStatus source_workspace_fail(SourceWorkspace *, SourceWorkspaceStatus, const char *, int, SourceMaterialTreeStatus);
/** Map an observed tree failure without inventing an errno-based failure stage. */
SourceWorkspaceStatus source_workspace_tree_status(const SourceMaterialTree *);
/** Normalize/open the explicit parent and match the retained no-follow identity. */
SourceWorkspaceStatus source_workspace_parent(SourceWorkspace *, const char *);
/** Select one unoccupied disjoint component; create nothing during selection. */
SourceWorkspaceStatus source_workspace_choose(SourceWorkspace *, const char *const *, size_t);
/** Match the opened root descriptor and its actual parent name to creation ownership. */
SourceWorkspaceStatus source_workspace_root(const SourceWorkspace *, int directory, SourceWorkspaceIssue *);
/** Verify each created child prefix and exact known root membership, including
 * failed prefixes; no ready flag is fabricated or required for cleanup. */
SourceWorkspaceStatus source_workspace_check(SourceWorkspace *, SourceWorkspaceIssue *);
/** Encode an exact view into owned text before filesystem file preparation. */
SourceWorkspaceStatus source_workspace_encode(SourceWorkspace *, const char *, const DeliverySourceSnapshotView *, const SourceText **);
#endif
