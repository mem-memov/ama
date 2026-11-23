/** @file
 * @brief Propose only missing destination module parents using the accepted positional correspondence.
 * [TOOL-01](../../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * retains existing parents, counts occupied positions and refuses absent or
 * conflicting source ancestors; [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * still requires the independent complete image/reference certificate.
 */
#ifndef AMA_SOURCE_COPY_CONTEXT_PLACEMENT_H
#define AMA_SOURCE_COPY_CONTEXT_PLACEMENT_H
#include "../ancestors.h"
#include "../../closure.h"
#include "../../../mapping/map.h"
#include "../../../mapping/identity.h"
#include "../../../observation/observation.h"
#include "../../../removal/selection/selection.h"
/** Candidate placement failures remain separate from final proof failures. */
typedef enum { SOURCE_COPY_PARENT_OK, SOURCE_COPY_PARENT_INVALID,
    SOURCE_COPY_PARENT_MEMORY, SOURCE_COPY_PARENT_IDENTITY, SOURCE_COPY_PARENT_MISSING,
    SOURCE_COPY_PARENT_UNOBSERVED, SOURCE_COPY_PARENT_KIND, SOURCE_COPY_PARENT_CONFLICT,
    SOURCE_COPY_PARENT_MAP } SourceCopyParentStatus;
/** Borrow genuine original/observed inputs and an open working C. All explicit
 * and active placements must already be present. Proposed context rows and owned
 * strings append to C's arena; this operation never seals it or grants authority.
 * Optional removal is the actual admitted original cut, not a final graph. */
typedef struct {
    const SourceCopyClosure *copy;
    SourceCloneMap *mains;
    const SourceObservedImage *observed;
    const SourceRemovalSelection *removal;
    const ModuleRoot *roots;
    size_t root_count;
} SourceCopyParentInputs;
/** Failure witnesses borrow inputs or C-arena paths through C destruction. */
typedef struct {
    const SourceCloneMapping *requester;
    const ModuleSource *required;
    const char *path;
    SourceIdentityStatus identity_status;
    SourceCloneStatus map_status;
} SourceCopyParentIssue;
/** Add each required absent parent from the corresponding original ancestor.
 * Derive destination positions from actual plus-storage components under the
 * exact supplied roots; ordinary directories consume no position. Existing
 * unchanged regular mains and independently explicit/active C rows occupy their
 * positions. Two context causes must demand the same original for a shared
 * missing parent. Refuse unknown observations, missing original positions and
 * conflicting mappings; do not search another ancestor or move an active copy.
 * No I/O/execution/source mutation. A failure after admission poisons sealing
 * while retaining all owned prefixes and exact child witnesses. Invalid admission
 * leaves an unrelated/ready map unchanged. Empty valid maps succeed. Work is
 * bounded by finite C rows, physical path components, original units/roles and
 * observation rows; injectivity limits added rows to original unit count.
 * Scratch consists of C-owned paths/rows. Final material, context, compiler,
 * reference, removal, configuration and transaction gates remain mandatory.
 */
SourceCopyParentStatus source_copy_context_place(const SourceCopyParentInputs *, SourceCopyParentIssue *);
#endif
