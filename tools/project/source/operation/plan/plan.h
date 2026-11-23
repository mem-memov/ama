/** @file
 * @brief Own a prepared copy/deletion image with explicit final semantic roots and independently checked package scopes.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * joins all intended writes before semantic admission and recoverable publication;
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * checks the combined original removal cut before preparing the simultaneous result.
 */
#ifndef AMA_SOURCE_OPERATION_PLAN_H
#define AMA_SOURCE_OPERATION_PLAN_H
#include "../../copy/image/image.h"
#include "../../copy/proof/proof.h"
#include "../../copy/configuration/check.h"
#include "../../removal/selection/selection.h"
#include "../../transaction/forest/forest.h"
/** An authorized prepared exact root set for one final physical package scope.
 * All three owners remain immutable and outlive plan destruction/proof. */
typedef struct {
    const SourcePackageEnvironment *environment;
    const SourcePackageScope *scope;
    SourceSelectionRoots roots;
} SourceOperationConfigurationExpectation;
/** Borrow fully prepared maps, edits, structural changes and original observations.
 * removal is the actual physical removal union, not a dependency closure. Each
 * supplemental REMOVE must belong to that union and every union path must occur.
 * Configuration edits/initialization and any other supplemental changes must have
 * their own prior authorization. final_roots is the exact final semantic selection,
 * including an explicitly empty set; it borrows immutable ModuleRoot rows.
 * original is the exact admitted analysis of observed->original. A copy supplies
 * its real closure/context owners. A deletion supplies no copy/context/texts and
 * empty sealed main/file maps plus a genuine removal selection; no synthetic copy
 * closure is accepted. The builder does not derive or authorize root-setting changes.
 * Expectations account for every final package.
 */
typedef struct {
    const ModuleRoot *final_roots;
    size_t final_root_count;
    const SourceAnalysis *original;
    const SourceCopyClosure *copy;
    const SourceObservedImage *observed;
    const SourceCloneMap *mains;
    const SourceFileMap *files;
    const SourceGraphPreparedText *texts;
    const SourceCopyContextOmissions *contexts;
    const SourceImageChange *changes;
    size_t change_count;
    SourceRemovalSelection *removal;
    const SourceRemovalInputUse *inputs;
    const SourceClientRoot *clients;
    const SourceOperationConfigurationExpectation *configurations;
    size_t configuration_count;
} SourceOperationPlanInputs;
/** Preserve exact failed preparation boundary instead of guessing from messages. */
typedef enum { SOURCE_OPERATION_PLAN_OK, SOURCE_OPERATION_PLAN_INVALID, SOURCE_OPERATION_PLAN_MODE,
    SOURCE_OPERATION_PLAN_REMOVAL, SOURCE_OPERATION_PLAN_IMAGE, SOURCE_OPERATION_PLAN_CAPTURE,
    SOURCE_OPERATION_PLAN_ENVIRONMENT, SOURCE_OPERATION_PLAN_CONFIGURATION, SOURCE_OPERATION_PLAN_MUTATIONS } SourceOperationPlanStatus;
/** Own image rows, final capture and independent final package environment.
 * Borrow all input owners until destruction. prepared proves read-only image/
 * cut/configuration preparation only; compiler/proof, freshness and recoverable
 * application remain mandatory. Never copy a live plan. */
typedef struct {
    SourceArena arena;
    SourceImageCapture capture;
    SourcePackageEnvironment environment;
    SourceOperationPlanInputs inputs;
    const SourceCatalogImage *image;
    SourceMutationForest mutations;
    SourceForestStatus mutation_status;
    SourceForestIssue mutation_issue;
    SourceRemovalReview removal_review;
    SourceRemovalStatus removal_status;
    SourceCopyImageIssue image_issue;
    SourceConfigurationCheckIssue configuration_issue;
    SourceConfigurationCheckStatus configuration_status;
    const SourcePackageScope *failed_scope;
    const char *failed_path;
    SourceOperationPlanStatus status;
    bool attempted, prepared;
} SourceOperationPlan;
/** Initialize child owners without I/O. */
void source_operation_plan_init(SourceOperationPlan *, SourceAllocator);
/** Destroy final environment/capture before rows and borrowed originals expire. */
void source_operation_plan_destroy(SourceOperationPlan *);
/** Build once from valid exact original observations and sealed maps. Enforce
 * explicit dependency mode for every ordinary C row that changes version, even
 * when existing permissions suffice. Review the physical removal union once,
 * verify its observed bytes/kinds and exact supplemental removal accounting,
 * then assemble/capture the simultaneous image and rebuild all package scopes.
 * Every actual final scope requires exactly one valid supplied expectation;
 * extra/missing/duplicate or unequal root sets refuse. Derive complete disjoint
 * mutation trees with stable observed parents for later staging. Publish prepared only
 * after all gates. Failure retains destructible typed child evidence.
 *
 * No source I/O, compiler/test/native execution or mutation. Inputs/supplemental
 * edit authority and closure/P completeness remain caller obligations. Finite
 * input scans plus documented child bounds terminate; arena and nested owners
 * retain projected image/catalog/environment allocations and failed prefixes.
 * Root-setting changes are not derived or authorized by this stage;
 * supply those through their explicit semantic/physical domain owners
 * before applying a general source operation.
 */
SourceOperationPlanStatus source_operation_plan_build(SourceOperationPlan *, const SourceOperationPlanInputs *);
/** Admit an independently successful client for this exact final capture through
 * the shared complete material/unit/site proof. No I/O/source mutation; repeatable. This stage
 * derives selected-registry retirement independently from original/final main
 * paths, while strict removal safety still uses the physical-removal union.
 * Complete root/client-setting authorization remains outside this builder. Success is
 * still not freshness, supplemental edit authorization or publication authority.
 */
SourceCopyProofStatus source_operation_plan_prove(SourceOperationPlan *, const SourceCompilerClient *, SourceCopyProofIssue *);
#endif
