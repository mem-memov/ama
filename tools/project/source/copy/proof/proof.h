/** @file
 * @brief Join actual final copy material, identity, compiler and occurrence proofs.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves originals and exact mapped copies, including drafts with no graph edges;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires semantic evidence for the same prepared inputs before publication.
 */
#ifndef AMA_SOURCE_COPY_PROOF_H
#define AMA_SOURCE_COPY_PROOF_H
#include "../contexts/omissions.h"
#include "../material/material.h"
#include "../../mapping/correspondence.h"
/** Separate malformed ownership, semantic input mismatch and failed copy laws. */
typedef enum { SOURCE_COPY_PROOF_OK, SOURCE_COPY_PROOF_INVALID,
    SOURCE_COPY_PROOF_INPUTS, SOURCE_COPY_PROOF_CORRESPONDENCE,
    SOURCE_COPY_PROOF_MATERIAL, SOURCE_COPY_PROOF_PLACEMENT,
    SOURCE_COPY_PROOF_ORIGINAL, SOURCE_COPY_PROOF_CONTEXT,
    SOURCE_COPY_PROOF_SITES } SourceCopyProofStatus;
/** Borrow first failed physical/unit witnesses and retain exact child diagnostics. */
typedef struct {
    const char *path;
    const SourceCapturedUnit *unit;
    SourceCorrespondenceIssue correspondence;
    SourceMaterialStatus material_status;
    SourceMaterialIssue material;
    SourceCopyContextIssue context;
    SourceSitesIssue sites;
} SourceCopyProofIssue;
/** Require admitted original analysis of C's exact capture, sealed C/P, a ready
 * complete context ledger over C, valid immutable final capture and successful
 * compiler response matching that capture's exact input bytes. Removed names
 * describe the previously reviewed union of original removals.
 *
 * Rebuild exact prepared material through its production join, compare every P
 * output against the actual final image, and require each C main at its declared
 * final identity/path. Retained originals preserve main path and complete dot
 * companion sets/bytes. Every final main is a retained original or a C image;
 * copied companions must belong to P, preventing leftover replacement material.
 * Then prove complete context obligations and original/copied graph/sites.
 * Drafts and edge-free definitions remain subject to physical checks.
 *
 * No I/O, execution or mutation. Issue resets on entry and borrows failed witnesses.
 * Scratch is arena-owned, including failed prefixes. Finite original/final/map/
 * material scans and child bounds terminate; beyond children O(U²+UF+UM+FM)
 * comparisons plus compared bytes, O(F) material scratch for U units, F files,
 * M image/map rows. This does not authorize removals, configuration changes,
 * dependency mode or publication: exact package-set, safe original cut, freshness
 * and recoverable transaction gates remain mandatory outer obligations.
 */
SourceCopyProofStatus source_copy_prove(SourceArena *, const SourceAnalysis *,
    const SourceCloneMap *, const SourceFileMap *, const SourceGraphPreparedText *,
    const SourceCopyContextOmissions *, const SourceImageCapture *, const SourceCompilerClient *,
    const char *const *removed, size_t, SourceCopyProofIssue *);
#endif
