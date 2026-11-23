/** @file
 * @brief Own a root-preserving prepared copy image and independently checked package scopes.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * joins all intended writes before semantic admission and recoverable publication;
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * checks the combined original removal cut before preparing any copy result.
 */
#ifndef AMA_SOURCE_COPY_PLAN_H
#define AMA_SOURCE_COPY_PLAN_H
#include "../image/image.h"
#include "../proof/proof.h"
#include "../configuration/check.h"
#include "../../removal/selection/selection.h"
/** An authorized prepared exact root set for one final physical package scope.
 * All three owners remain immutable and outlive plan destruction/proof. */
typedef struct {
    const SourcePackageEnvironment *environment;
    const SourcePackageScope *scope;
    SourceSelectionRoots roots;
} SourceCopyConfigurationExpectation;
/** Borrow fully prepared maps, edits, structural changes and original observations.
 * removal is the actual physical removal union, not a dependency closure. Each
 * supplemental REMOVE must belong to that union and every union path must occur.
 * Configuration edits/initialization and any other supplemental changes must have
 * their own prior authorization. Expectations account for every final package.
 */
typedef struct {
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
    const SourceCopyConfigurationExpectation *configurations;
    size_t configuration_count;
} SourceCopyPlanInputs;
/** Preserve exact failed preparation boundary instead of guessing from messages. */
typedef enum { SOURCE_COPY_PLAN_OK, SOURCE_COPY_PLAN_INVALID, SOURCE_COPY_PLAN_MODE,
    SOURCE_COPY_PLAN_REMOVAL, SOURCE_COPY_PLAN_IMAGE, SOURCE_COPY_PLAN_CAPTURE,
    SOURCE_COPY_PLAN_ENVIRONMENT, SOURCE_COPY_PLAN_CONFIGURATION } SourceCopyPlanStatus;
/** Own image rows, final capture and independent final package environment.
 * Borrow all input owners until destruction. prepared proves read-only image/
 * cut/configuration preparation only; compiler/proof, freshness and recoverable
 * application remain mandatory. Never copy a live plan. */
typedef struct {
    SourceArena arena;
    SourceImageCapture capture;
    SourcePackageEnvironment environment;
    SourceCopyPlanInputs inputs;
    const SourceCatalogImage *image;
    SourceRemovalReview removal_review;
    SourceRemovalStatus removal_status;
    SourceCopyImageIssue image_issue;
    SourceConfigurationCheckIssue configuration_issue;
    SourceConfigurationCheckStatus configuration_status;
    const SourcePackageScope *failed_scope;
    const char *failed_path;
    SourceCopyPlanStatus status;
    bool attempted, prepared;
} SourceCopyPlan;
/** Initialize child owners without I/O. */
void source_copy_plan_init(SourceCopyPlan *, SourceAllocator);
/** Destroy final environment/capture before rows and borrowed originals expire. */
void source_copy_plan_destroy(SourceCopyPlan *);
/** Build once from valid exact original observations and sealed maps. Enforce
 * explicit dependency mode for every ordinary C row that changes version, even
 * when existing permissions suffice. Review the physical removal union once,
 * verify its observed bytes/kinds and exact supplemental removal accounting,
 * then assemble/capture the simultaneous image and rebuild all package scopes.
 * Every actual final scope requires exactly one valid supplied expectation;
 * extra/missing/duplicate or unequal root sets refuse. Publish prepared only
 * after all gates. Failure retains destructible typed child evidence.
 *
 * No source I/O, compiler/test/native execution or mutation. Inputs/supplemental
 * edit authority and closure/P completeness remain caller obligations. Finite
 * input scans plus documented child bounds terminate; arena and nested owners
 * retain projected image/catalog/environment allocations and failed prefixes.
 * Root-setting changes and final effective-registry retirement for promotion are
 * not derived by this root-preserving stage; supply those through their owners
 * before applying a general source operation.
 */
SourceCopyPlanStatus source_copy_plan_build(SourceCopyPlan *, const SourceCopyPlanInputs *);
/** Admit an independently successful client for this exact final capture through
 * the complete final-copy proof. No I/O/source mutation; repeatable. This stage
 * uses actual physical-removal names as original registry retirement names;
 * a promotion/selection change needs an explicitly prepared effective-registry
 * retirement set before this interface can serve that composition. Success is
 * still not freshness, supplemental edit authorization or publication authority.
 */
SourceCopyProofStatus source_copy_plan_prove(SourceCopyPlan *, const SourceCompilerClient *, SourceCopyProofIssue *);
#endif
