/** @file
 * @brief Admit unchanged formerly ignored suppliers after independent final version selection.
 * [TOOL-02](../../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * applies [PKG-02](../../../../../../spec/project/packages.md#pkg-02) to both views
 * while protecting every surviving original binding.
 */
#ifndef AMA_SOURCE_COPY_INTRODUCTION_H
#define AMA_SOURCE_COPY_INTRODUCTION_H
#include "../../../observation/observation.h"
#include "../../../mapping/map.h"
#include "../../../mapping/identity.h"
#include "../../../analysis/analysis.h"
#include "../../../capture/image.h"
#include "../../registry/retirement.h"
/** Distinguish absent provenance, changed bytes, rebound targets and allocation refusal. */
typedef enum { SOURCE_INTRODUCTION_OK, SOURCE_INTRODUCTION_INVALID, SOURCE_INTRODUCTION_ORIGIN,
    SOURCE_INTRODUCTION_MATERIAL, SOURCE_INTRODUCTION_TARGET, SOURCE_INTRODUCTION_MEMORY } SourceIntroductionStatus;
/** Arena-owned pointer array and conserved final graph rows; all contents borrow
 * the actual observed/final captures and checked compiler responses. Never marks
 * an ignored source as a selected original or manufactures compiler admission. */
typedef struct {
    const SourceCapturedUnit **units;
    size_t count;
    const DeliverySourceDefinition *conserved;
} SourceCopyIntroduction;
/** Borrow the exact failed unit/path or original edge for diagnostics. */
typedef struct {
    const SourceCapturedUnit *unit;
    const char *path;
    const DeliverySourceDependency *edge;
    SourceIdentityStatus identity_status;
} SourceIntroductionIssue;
/** Derive final units outside retained originals/C only when the original roots
 * already cover the same ordinary identity, a different copy of that complete
 * version was originally selected, and every final main/dot companion and package
 * configuration has unchanged explicitly observed original bytes and membership.
 * Final common discovery/compiler admission is mandatory at the caller. Reject
 * rebinding a conserved original/copied occurrence to an introduced supplier.
 * Restrict the checked final graph to retained/C authors for the existing exact
 * conservation proof; introduced authors have no selected original edge baseline.
 * No I/O, mutation, source augmentation or publication authority. Failure leaves
 * output unchanged and arena prefixes owned. For F final/U original units,
 * C clones, Q retired names, J introductions, V original versions, B image rows,
 * K maximum unit membership, H final/O original definitions and E inspected
 * original edges, comparisons are bounded by
 * O(F(C+U+Q) + J(V+BK) + H(O+C+J) + E(J+C)), plus two common placement
 * operations per introduction. Owned scratch is O(J+H) rows plus their shared
 * placement-path scratch. Borrow captures/responses through all consumers.
 */
SourceIntroductionStatus source_copy_introduction_prepare(SourceArena *, const SourceObservedImage *,
    const SourceImageCapture *, const SourceCloneMap *, SourceRegistryRetirement,
    const DeliverySourceDefinition *, const DeliverySourceDefinition *,
    SourceCopyIntroduction *, SourceIntroductionIssue *);
/** Exact nominal membership in the independently certified introduction set. */
bool source_copy_introduction_contains(const SourceCopyIntroduction *, const char *);
#endif
