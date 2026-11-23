/** @file
 * @brief Account explicitly for configured, augmented-original and final root settings.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * preserves carried root kind and reports settings without inventing configuration;
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * analyzes newly covered inputs before edits; [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates settings, semantic selection and physical removal.
 */
#ifndef AMA_SOURCE_OPERATION_ROOTS_H
#define AMA_SOURCE_OPERATION_ROOTS_H
#include "../plan/plan.h"
#include "../../mapping/identity.h"
/** A removed setting does not authorize removal of any physical files. */
typedef enum { SOURCE_OPERATION_ROOT_KEEP, SOURCE_OPERATION_ROOT_DROP,
    SOURCE_OPERATION_ROOT_CARRY, SOURCE_OPERATION_ROOT_ADD } SourceOperationRootAction;
/** One client choice per original configured root, in its original order; ADD
 * is a report action reserved for explicit destination additions.
 * KEEP keeps path/kind but can explicitly revise exclusions. CARRY names an
 * actual P directory destination and preserves kind. DROP uses SIZE_MAX final. */
typedef struct { SourceOperationRootAction action; size_t final; } SourceOperationRootDisposition;
/** Account for a final setting not reached by an original disposition.
 * source == SIZE_MAX is an explicit destination-root choice. Otherwise the
 * source is an original configured root copied through an actual P directory.
 * Keeping that original and adding its copy is therefore expressible directly. */
typedef struct { size_t final, source; } SourceOperationRootAddition;
/** Borrow genuine captures and explicit client choices. configured is the O
 * snapshot before augmentation. plan's original observation is the single common
 * U catalog, never a merger of separate analyzed registries; its final image is F.
 * Valid captures/plan, their root descriptors and arrays remain immutable through
 * destruction. No opaque external root-config file is presumed writable. */
typedef struct {
    const SourceRootCapture *configured;
    const SourceOperationPlan *plan;
    const SourceOperationRootDisposition *originals;
    size_t original_count;
    const SourceOperationRootAddition *additions;
    size_t addition_count;
} SourceOperationRootsInputs;
/** Every report row corresponds to an explicit choice, not an inferred filesystem change. */
typedef struct SourceOperationRootSetting {
    struct SourceOperationRootSetting *next;
    const ModuleRoot *original, *final;
    SourceOperationRootAction action;
    bool addition, explicit_destination;
} SourceOperationRootSetting;
/** Preserve root accounting, missing original analysis and identity/material refusals. */
typedef enum { SOURCE_OPERATION_ROOTS_OK, SOURCE_OPERATION_ROOTS_INVALID,
    SOURCE_OPERATION_ROOTS_ACCOUNTING, SOURCE_OPERATION_ROOTS_KIND,
    SOURCE_OPERATION_ROOTS_PLACEMENT, SOURCE_OPERATION_ROOTS_COVERAGE,
    SOURCE_OPERATION_ROOTS_ORIGINAL, SOURCE_OPERATION_ROOTS_IDENTITY,
    SOURCE_OPERATION_ROOTS_MEMORY } SourceOperationRootsStatus;
/** Own finite report/path scratch and borrow all immutable input facts. valid
 * proves settings/analysis coverage only, not closure, final graph, freshness or
 * publication. original configured capture may equal the augmented capture. */
typedef struct {
    SourceArena arena;
    SourceOperationRootsInputs inputs;
    SourceOperationRootSetting *first, *last;
    const char *failed_path;
    SourceIdentityStatus identity_status;
    SourceOperationRootsStatus status;
    bool attempted, valid;
} SourceOperationRoots;
/** Initialize empty effect-free ownership. */
void source_operation_roots_init(SourceOperationRoots *, SourceAllocator);
/** Retire every success/failure prefix before borrowed captures expire. */
void source_operation_roots_destroy(SourceOperationRoots *);
/** Check once. Require exact accounting of all O/F settings, real directory P
 * evidence for carries and unchanged root family. U must physically cover every
 * configured region and every preexisting final region, accounting exactly for
 * root/exclusion boundaries. Proven absent final roots require no fictitious
 * original capture. Preserve already covered module/version identities and
 * original complete bytes against U's observations. U itself must be the plan's
 * genuine independently analyzed original capture; all final proof gates remain
 * mandatory. No I/O, source mutation or implicit deletion/configuration edits.
 * Finite boundary scans decide subtree-minus-exclusion inclusion; no sampling,
 * filesystem enumeration or path-depth limit is used. Report storage is O(O+A),
 * path/identity scratch is bounded by the total explicitly examined spelling
 * lengths. Work is pairwise root/exclusion coverage, configured units/versions
 * times U roots/image entries, and (O+A)*P placement/accounting scans.
 */
SourceOperationRootsStatus source_operation_roots_build(SourceOperationRoots *, const SourceOperationRootsInputs *);
#endif
