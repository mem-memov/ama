/** @file
 * @brief Record original context omitted provisionally and prove its final topology separately.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * permits unchanged destination parents without copying old context. A candidate
 * file relation alone does not establish the final parent or its provenance.
 */
#ifndef AMA_SOURCE_COPY_CONTEXT_OMISSIONS_H
#define AMA_SOURCE_COPY_CONTEXT_OMISSIONS_H
#include "../closure.h"
#include "../../mapping/map.h"
#include "../../capture/image.h"
/** A missing final provider and a changed retained parent are distinct refusals. */
typedef enum { SOURCE_COPY_CONTEXT_OK, SOURCE_COPY_CONTEXT_INVALID,
    SOURCE_COPY_CONTEXT_MEMORY, SOURCE_COPY_CONTEXT_MISSING,
    SOURCE_COPY_CONTEXT_CHANGED } SourceCopyContextStatus;
/** One borrowed structural-only original role; arena owns the list node. */
typedef struct SourceCopyContextOmission {
    struct SourceCopyContextOmission *next;
    const AcquisitionRoleRecord *role;
} SourceCopyContextOmission;
/** Borrow immutable copy/ready main map; own only unique omission metadata.
 * ready freezes a complete obligation list, not a successful final proof.
 */
typedef struct {
    SourceArena arena;
    const SourceCopyClosure *copy;
    const SourceCloneMap *mains;
    SourceCopyContextOmission *first;
    SourceCopyContextStatus status;
    bool ready;
} SourceCopyContextOmissions;
/** Borrow exact original context/requiring role and final supplying parent. */
typedef struct {
    const AcquisitionRoleRecord *context, *requester;
    const SourceCapturedUnit *parent;
} SourceCopyContextIssue;
/** Begin one obligation owner; both immutable input owners outlive it. */
void source_copy_context_omissions_init(SourceCopyContextOmissions *, SourceAllocator,
    const SourceCopyClosure *, const SourceCloneMap *);
/** Retire obligation metadata before either borrowed input expires. */
void source_copy_context_omissions_destroy(SourceCopyContextOmissions *);
/** Record a genuine unmapped context UNIT_MEMBER requester from this exact copy.
 * Reject active/foreign/mapped roles. Equal repeats allocate nothing; failed
 * scratch/prefix remains owned and unsealable. No I/O or material mutation.
 */
SourceCopyContextStatus source_copy_context_omit(SourceCopyContextOmissions *, const AcquisitionRoleRecord *);
/** Freeze only if every unmapped unit cause is context with a recorded obligation;
 * missing active mains always refuse. This proves complete accounting, not that
 * the omitted context is unnecessary in a valid final tree.
 */
SourceCopyContextStatus source_copy_context_omissions_seal(SourceCopyContextOmissions *);
/** Prove frozen obligations against actual common final discovery. Every requiring
 * copied main must exist at its mapped identity/path. Its actual plus ancestors
 * must either be explicitly mapped mains or unchanged retained originals with
 * identical main/dot-companion bytes and no removal entry. Ordinary containing
 * directories need no parent main. No old parent becomes a new reference target.
 *
 * No allocation/I/O/mutation; repeatable. Output issue is cleared on success.
 * Final compiler, C/P correspondence, graph/site laws, freshness and safe removal
 * remain separate mandatory gates: this proves only context omission/topology.
 * Finite role/reason/main/ancestor/file scans bound work; constant scratch.
 */
SourceCopyContextStatus source_copy_context_omissions_check(const SourceCopyContextOmissions *,
    const SourceImageCapture *, const char *const *removed, size_t, SourceCopyContextIssue *);
#endif
