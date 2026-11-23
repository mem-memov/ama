/** @file
 * @brief Construct exact original removal material without requiring missing tests or cloning dependencies.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * removes actually present associated material;
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * checks all original surviving consumers and explicit client roots.
 * Final topology, freshness and transaction validation remain separate gates.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires their completion before any source mutation.
 */
#ifndef AMA_PROJECT_SOURCE_REMOVAL_SELECTION_H
#define AMA_PROJECT_SOURCE_REMOVAL_SELECTION_H
#include "../../selection/domain.h"
#include "../../analysis/analysis.h"
#include "../../inputs/declared.h"
#include "../roots.h"
/** Preserve every explicit reason for original physical membership. */
typedef enum { SOURCE_REMOVAL_RAW_TREE, SOURCE_REMOVAL_UNIT,
    SOURCE_REMOVAL_ASSOCIATED_TEST, SOURCE_REMOVAL_DECLARED_INPUT } SourceRemovalGroupKind;
/** Retain raw, complete-unit, associated and declared groups separately. */
typedef struct SourceRemovalGroup {
    struct SourceRemovalGroup *next;
    const AcquisitionUnitEntry *files;
    const SourceCapturedUnit *requester;
    const AcquisitionTestDirectory *association;
    SourceRemovalGroupKind kind;
} SourceRemovalGroup;
/** One unique captured physical member; overlaps must agree in shape and bytes. */
typedef struct SourceRemovalSelectedFile {
    struct SourceRemovalSelectedFile *next;
    const AcquisitionUnitEntry *file;
} SourceRemovalSelectedFile;
/** Material preparation failures never certify a safe deletion. */
typedef enum { SOURCE_REMOVAL_SELECTION_OK, SOURCE_REMOVAL_SELECTION_INVALID,
    SOURCE_REMOVAL_SELECTION_DOMAIN, SOURCE_REMOVAL_SELECTION_FILES,
    SOURCE_REMOVAL_SELECTION_CHANGED, SOURCE_REMOVAL_SELECTION_MEMORY } SourceRemovalSelectionStatus;
/** Own additional associated captures and exact material indices; borrow raw tree,
 * original capture and admitted compiler response through all review consumption.
 * valid means complete original removal material only, not allowed publication.
 */
typedef struct {
    SourceArena arena;
    SourceManager files;
    const SourceTreeSelection *tree;
    SourceAnalysis *analysis;
    SourceRemovalGroup *groups;
    SourceRemovalSelectedFile *selected;
    SourceRemovalInputUse *inputs;
    struct SourceRemovalProcessed *processed;
    const char *const *paths, *const *identities;
    size_t path_count, identity_count;
    const char *failed_path;
    SourceTreeDomainStatus domain_status;
    SourcePhysicalIssue physical_issue;
    SourceStatus file_status;
    SourceRemovalSelectionStatus status;
    bool attempted, valid;
} SourceRemovalSelection;
/** Separate authored/input and explicit client-root blockers; both remain original facts. */
typedef struct {
    const SourceRemovalBlocker *consumers;
    const SourceClientRootBlocker *clients;
} SourceRemovalReview;
/** Begin owned material preparation without filesystem or compiler work. */
void source_removal_selection_init(SourceRemovalSelection *, SourceAllocator);
/** Retire review/material metadata before associated bytes and borrowed original owners. */
void source_removal_selection_destroy(SourceRemovalSelection *);
/** Build once from an admitted raw domain and the exact same original compiler
 * view. Include only actually present associated directories of active ordinary
 * src seeds; no concrete-test admission or outgoing dependency expansion occurs.
 * Test helpers and drafts add no tests-of-tests. Include the supplied explicit
 * input groups of removed owners, retaining outside shared consumers
 * for the subsequent review. Merge exact physical membership,
 * preserving all groups, and include every removed original main in the identity
 * set, including drafts/open helpers. Close finite physical unit/test/declared
 * membership once per original main; do not follow outgoing module dependencies.
 * Refuse contradictory observations; no
 * recapture fallback or source mutation. Partial failure is owned, never valid.
 */
SourceRemovalSelectionStatus source_removal_selection_build(SourceRemovalSelection *,
    const SourceTreeSelection *, SourceAnalysis *, const SourceDeclaredGroup *);
/** Union one or more valid selections from exactly the same capture and analysis
 * into a fresh initialized selection. Borrow every contributing selection until
 * the merged selection and all its consumers retire; groups and input-list nodes
 * are copied, while observed bytes and occurrence owners remain borrowed.
 * Reuse the exact physical index: repeated paths must agree in kind and bytes;
 * identities are derived from that union, including drafts. Deduplicate identical
 * original input occurrences by owner, path, span and exact origin bytes, never
 * just by target path. Review the union once, not each part independently.
 * No I/O, dependency expansion, source mutation or final-graph authorization.
 * Invalid admission leaves the fresh output unattempted; allocation/observation
 * failure leaves an owned invalid prefix for destroy, never a valid partial set.
 * Finite group/file/input scans terminate; with F physical entries, I uses and
 * U original units, time is O(F² + I² + UF + U²) comparisons and space O(F+I+U)
 * plus retained group reasons. Original cuts, final proof and publication remain
 * separate requirements under [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03).
 */
SourceRemovalSelectionStatus source_removal_selection_merge(SourceRemovalSelection *,
    const SourceRemovalSelection *const *, size_t);
/** Check both original graph/input and client-root cuts against this exact set.
 * Captured configuration/native inputs and all supplied declaration uses are
 * included automatically for active original owners. inputs adds separately
 * identified external uses, without replacing those known original requirements.
 * Publish review only after both predicates complete; BLOCKED reports every
 * occurrence in both categories. Explicit roots and declared input records borrow
 * their caller owners through consumption. This necessary safety review does not
 * establish final structural topology or transaction freshness under
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04).
 */
SourceRemovalStatus source_removal_selection_review(SourceRemovalSelection *, const SourceRemovalInputUse *,
    const SourceClientRoot *, SourceRemovalReview *);
#endif
