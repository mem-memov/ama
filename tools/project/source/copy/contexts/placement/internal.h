/** @file
 * @brief Separate positional parent scanning from observed/planned provider admission.
 * [TOOL-01](../../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * keeps original positional correspondence independent of discovery order.
 */
#ifndef AMA_SOURCE_COPY_CONTEXT_PLACEMENT_INTERNAL_H
#define AMA_SOURCE_COPY_CONTEXT_PLACEMENT_INTERNAL_H
#include "placement.h"
/** Join one decoded destination parent at its exact position; mutate only C.
 * Inputs are already admitted by the outer orchestration. */
SourceCopyParentStatus source_copy_parent_provider(const SourceCopyParentInputs *, const SourceCloneMapping *,
    const char *, const char *, size_t, SourceCopyParentIssue *);
/** Decode one actual C main, then visit its strict plus parents nearest first.
 * All scratch paths remain owned by the open relation on failure. */
SourceCopyParentStatus source_copy_parent_scan(const SourceCopyParentInputs *, const SourceCloneMapping *, SourceCopyParentIssue *);
#endif
