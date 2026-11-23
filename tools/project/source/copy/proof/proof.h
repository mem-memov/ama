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
#include "../registry/retirement.h"
#include "introduction/introduction.h"
/** Separate malformed ownership, semantic input mismatch and failed copy laws. */
typedef enum { SOURCE_COPY_PROOF_OK, SOURCE_COPY_PROOF_INVALID,
    SOURCE_COPY_PROOF_INPUTS, SOURCE_COPY_PROOF_CORRESPONDENCE,
    SOURCE_COPY_PROOF_MATERIAL, SOURCE_COPY_PROOF_PLACEMENT,
    SOURCE_COPY_PROOF_ORIGINAL, SOURCE_COPY_PROOF_CONTEXT,
    SOURCE_COPY_PROOF_SITES, SOURCE_COPY_PROOF_REGISTRY, SOURCE_COPY_PROOF_EDITS, SOURCE_COPY_PROOF_INTRODUCTION } SourceCopyProofStatus;
/** Borrow first failed physical/unit witnesses and retain exact child diagnostics. */
typedef struct {
    const char *path;
    const SourceCapturedUnit *unit;
    SourceCorrespondenceIssue correspondence;
    SourceMaterialStatus material_status;
    SourceMaterialIssue material;
    SourceCopyContextIssue context;
    SourceSitesIssue sites;
    SourceRegistryStatus registry_status;
    SourceIntroductionStatus introduction_status;
    SourceIntroductionIssue introduction;
    const SourceGraphPreparedText *prepared;
    const SourceTextEdit *edit;
} SourceCopyProofIssue;
/** Require admitted original analysis of C's exact capture, sealed C/P,
 * valid immutable final capture and successful
 * compiler response matching that capture's exact input bytes. Copies additionally
 * require a ready complete context ledger over C. Removed names
 * describe only the previously reviewed union of actual physical removals.
 * Derive selected-registry retirement separately from original/final main paths.
 *
 * Rebuild exact prepared material through its production join, compare every P
 * output against the actual final image, and require each C main at its declared
 * final identity/path. Retained originals preserve main path and complete dot
 * companion sets/bytes. An ignored original not physically removed must still
 * preserve its complete captured unit in the image; retirement cannot excuse
 * lost/changed bytes merely because the selected final view compiles.
 * Every final main is a retained original, C image, or certified untouched
 * formerly ignored supplier after independent PKG-02 selection. Such suppliers
 * require the same identity under original roots, an originally selected other
 * copy of their version, complete original observed main/companion/configuration
 * preservation, and final common compiler admission. Their final authors have no
 * original selected edge baseline; conserved original/copied edges may not change
 * target to them. The original observer must belong to C's exact capture.
 * For every C image,
 * copied companions must belong to P, preventing leftover replacement material.
 * Main-file edits must remain wholly inside explicit original module operands.
 * Zero-width insertions require strict interior positions: boundary-biased site
 * projection cannot authorize adding code before or after an operand. The final
 * whole projected sites must equal actual parsed sites, so a permitted edit
 * cannot inject unrelated code. Literals, comments, labels, storage suffixes and
 * implicit requiring constructs are not writable operands. Non-main configuration
 * edits and other supplemental writes still require their separate outer owners.
 * Then prove complete context obligations and original/copied graph/sites.
 * Drafts and edge-free definitions remain subject to physical checks.
 *
 * No I/O, execution or mutation. Issue resets on entry and borrows failed witnesses.
 * Scratch is arena-owned, including failed prefixes. Finite original/final/map/
 * material scans and child bounds terminate; beyond children O(U²+UF+UM+FM)
 * comparisons plus compared bytes and O(I*N) ignored-unit checks for I original
 * unit files and N image rows. Registry derivation has its own documented bound.
 * O(F+U) material/retirement scratch for U units, F files, M image/map rows.
 * This does not authorize removals, configuration changes,
 * dependency mode or publication: exact package-set, safe original cut, freshness
 * and recoverable transaction gates remain mandatory outer obligations.
 *
 * A deletion may omit contexts only with empty sealed C/P and no prepared
 * text. Empty relations introduce no copied context obligations; all original
 * units, registry retirement and final site laws still undergo the same checks.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * retains independent original-cut authorization at the enclosing plan boundary.
 */
SourceCopyProofStatus source_copy_prove(SourceArena *, const SourceAnalysis *, const SourceObservedImage *,
    const SourceCloneMap *, const SourceFileMap *, const SourceGraphPreparedText *,
    const SourceCopyContextOmissions *, const SourceImageCapture *, const SourceCompilerClient *,
    const char *const *removed, size_t, SourceCopyProofIssue *);
#endif
