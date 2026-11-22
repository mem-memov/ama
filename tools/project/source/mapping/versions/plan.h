/** @file
 * @brief Resolve explicitly constructed destination-version demands for copy units.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * makes additional dependencies follow their requiring unit's destination version
 * while explicit tree placements retain their own destinations. Conflicting
 * demands refuse instead of creating a second implicit clone.
 */
#ifndef AMA_PROJECT_SOURCE_VERSION_PLAN_H
#define AMA_PROJECT_SOURCE_VERSION_PLAN_H
#include "source/module/catalog.h"
/** One distinct original unit. fixed_version is a canonical ordinary version for
 * an explicit placement, or NULL when its destination must follow a requiring
 * unit. Original handles are borrowed; the outer planner certifies their catalog
 * membership and which units/flows actually belong to the copy operation.
 */
typedef struct {
    const ModuleSource *original;
    const char *fixed_version;
} SourceVersionUnit;
/** Exact directed requirement between indices in the supplied unit array.
 * A fixed target retains its explicit destination, including cross-version edges.
 * A nonfixed target receives its requester's destination. These are already
 * constructed demands, not inferred semantic/test/context ownership relations.
 */
typedef struct { size_t requester, target; } SourceVersionFlow;
/** Own version bytes in the caller's arena; original handles remain borrowed. */
typedef struct { const ModuleSource *original; const char *version; } SourceVersionAssignment;
/** Unresolved disconnected cycles are distinct from contradictory known demands. */
typedef enum { SOURCE_VERSION_OK, SOURCE_VERSION_INVALID, SOURCE_VERSION_CONFLICT,
    SOURCE_VERSION_UNRESOLVED, SOURCE_VERSION_MEMORY } SourceVersionStatus;
/** Borrow input labels as conflict witnesses; the input owners outlive inspection.
 * unit/flow are meaningful for CONFLICT; unit alone for UNRESOLVED. Other states
 * leave the cleared witness. This never grants final physical/catalog authority.
 */
typedef struct { size_t unit, flow; const char *first, *second; } SourceVersionIssue;
/** Solve the least directed demand closure over finite ordinary copy units.
 * Require unique non-NULL original handles and canonical ordinary module names;
 * fixed labels must be canonical version identities. Validate all flows before
 * propagation. Labels only change from unknown to one fixed input label; equal
 * repeats are idempotent, unequal labels conflict. A fixed target ignores incoming
 * version demands because its explicit mapping wins. Refuse any unknown at the
 * fixed point. No recursion, guessing or unbounded source expansion is involved.
 * Output is published only after every owned assignment/string succeeds. Empty
 * input succeeds with NULL output. Scratch, including failed prefixes, stays in
 * arena. No I/O or mutation; O(N squared + N*E) path comparisons plus copied bytes, and O(N + output
 * bytes) storage. This is a placement subproblem: the caller must construct flows
 * from the agreed closure and deepest test associations and still prove final
 * physical layout, injectivity, package selection and graph preservation.
 */
SourceVersionStatus source_version_plan(SourceArena *, const SourceVersionUnit *, size_t,
    const SourceVersionFlow *, size_t, const SourceVersionAssignment **, SourceVersionIssue *);
/** Build the candidate name for an additional ordinary src dependency or an
 * outside-associated-directory test helper under its resolved destination version.
 * Preserve the full original area-relative suffix. Do not use this operation to
 * override explicit tree placement or deepest associated-test mapping. Validate
 * both canonical endpoints, borrow them only during the call, and own the output
 * in arena. Output changes only on success; no physical plus layout, catalog
 * admission, injectivity or publication authority is implied. O(path bytes) time
 * and output storage; no filesystem/source reads or mutation.
 */
SourceStatus source_additional_module_identity(SourceArena *, const SourceVersionAssignment *, const char **output);

#endif
