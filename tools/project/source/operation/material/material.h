/** @file
 * @brief Reconstruct complete requested copy material from authoritative raw, unit, test and declared causes.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires complete P and exact additional ordinary destinations;
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) preserves opaque units;
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) preserves associated tests.
 */
#ifndef AMA_SOURCE_OPERATION_MATERIAL_H
#define AMA_SOURCE_OPERATION_MATERIAL_H
#include "../intent/intent.h"
#include "../../copy/destinations/destinations.h"
#include "../../copy/members/units.h"
#include "../../copy/members/declared.h"
#include "../../copy/tests/tests.h"
#include "../../copy/contexts/placement/placement.h"
/** Retain the exact failed owner/stage rather than infer meaning from diagnostics. */
typedef enum { SOURCE_OPERATION_MATERIAL_OK, SOURCE_OPERATION_MATERIAL_INVALID,
    SOURCE_OPERATION_MATERIAL_INTENT, SOURCE_OPERATION_MATERIAL_EXPLICIT,
    SOURCE_OPERATION_MATERIAL_DESTINATIONS, SOURCE_OPERATION_MATERIAL_LOGICAL, SOURCE_OPERATION_MATERIAL_RAW,
    SOURCE_OPERATION_MATERIAL_UNITS, SOURCE_OPERATION_MATERIAL_TESTS,
    SOURCE_OPERATION_MATERIAL_DECLARED, SOURCE_OPERATION_MATERIAL_FILESET,
    SOURCE_OPERATION_MATERIAL_MEMORY, SOURCE_OPERATION_MATERIAL_CONTEXT } SourceOperationMaterialStatus;
/** Own rederived relation/candidate evidence and all failed prefixes. Borrow the
 * immutable prepared plan, intent and nested original owners through teardown.
 * valid proves material completeness and active ordinary name placement only;
 * write/root/client authority, context necessity, final proof and publication
 * remain mandatory. Never copy a live owner.
 */
typedef struct SourceOperationMaterialReview {
    SourceArena arena;
    SourceCloneMap explicit_mains, expected_mains;
    SourceCopyDestinations destinations;
    SourceFileMap expected;
    SourceCopyContextOmissions omissions;
    SourceCopyParentStatus parent_status;
    SourceCopyParentIssue parent_issue;
    const SourceOperationPlan *plan;
    const SourceOperationIntent *intent;
    const SourceVersionPlacement *versions;
    size_t version_count;
    const char *failed_path;
    const ModuleSource *failed_main;
    SourceOperationIntentStatus intent_status;
    SourceOperationIntentIssue intent_issue;
    SourceCopyUnitsStatus units_status;
    SourceCopyUnitsIssue units_issue;
    SourceCopyTestsStatus tests_status;
    SourceCopyTestsIssue tests_issue;
    SourceCopyDeclaredStatus declared_status;
    SourceCopyDeclaredIssue declared_issue;
    SourceOperationMaterialStatus status;
    bool attempted, valid;
} SourceOperationMaterialReview;
/** Initialize independent arenas without I/O or source effects. */
void source_operation_material_init(SourceOperationMaterialReview *, SourceAllocator);
/** Retire expected relations and candidates before the borrowed original plan. */
void source_operation_material_destroy(SourceOperationMaterialReview *);
/** Review once, checking actual request intent before deriving any copy relation.
 * Deletion uses genuine empty relations; no copy closure is fabricated. Copies
 * derive explicit mains from exactly the raw selected original handles, active
 * ordinary destinations through the common solver and version storage from the
 * actual final package environment. Rebuild P with the shared raw/unit/test/data
 * operations and compare both set inclusions. Ignore no mapped extra or missing
 * required member. Independently reconstruct context C from raw/active placements
 * and the shared positional parent operation, comparing both map inclusions.
 * Context omission remains subject to the final context proof.
 *
 * No I/O, execution, source mutation or source reanalysis. Failed prefixes remain
 * owned/invalid; exact child statuses and witnesses remain inspectable. Finite
 * child algorithms plus O(C(R+D) + V + F squared) key/byte comparisons bound work,
 * for C clones, R raw mains, D derived candidates, V final versions and F relation
 * entries; owned
 * storage is the sum of rederived child bounds and O(V) version rows. Inputs
 * outlive review destruction. Success is not full operation authorization.
 */
SourceOperationMaterialStatus source_operation_material_build(SourceOperationMaterialReview *,
    const SourceOperationPlan *, const SourceOperationIntent *);
#endif
