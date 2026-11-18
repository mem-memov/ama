/**
 * @file
 * @brief Own binding snapshots independently of validated static lexical inventories.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) requires failed
 * allocations to remain owned. [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * snapshots actual residual inputs without mutating their producer metadata.
 * [CALL-06](../../../../spec/language/calls-and-results.md#call-06) preserves
 * independent runtime arities. [LIST-04](../../../../spec/language/lists.md#list-04)
 * supplies intrinsic interfaces from canonical immutable descriptors.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) keeps bounded
 * prior-binding and actual-declaration diagnostic contexts without allocation.
 * [TPL-09](../../../../spec/language/templates.md#tpl-09) selects shared constants.
 */
#ifndef AMA_BINDING_SEQUENCE_OWNER_H
#define AMA_BINDING_SEQUENCE_OWNER_H
#include "model.h"

/** Sticky failure prevents further construction; existing snapshots remain owned.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
typedef enum { BINDING_OK, BINDING_INVALID, BINDING_OUT_OF_MEMORY } BindingStatus;

/** Own only compile-time metadata/scratch and borrow source/intrinsic identities.
 * Destroy before any borrowed StaticInventory/Project expires. No global validity
 * flag substitutes for an individual operation's successful output publication.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
typedef struct {
    SourceArena arena;
    BindingInterface *first_interface;
    BindingInterface *last_interface;
    BindingApplication *first_application;
    BindingApplication *last_application;
    BindingStatus status;
    Diagnostic error;
    DiagnosticRelated related[2];
} BindingPlan;

/** Initialize without allocation; allocator authority outlives destruction.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void binding_plan_init(BindingPlan *, SourceAllocator);
/** Free successful/partial metadata without following borrowed identity pointers.
 * All views expire, the owner is zeroed, and source inventories remain unchanged.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void binding_plan_destroy(BindingPlan *);
/** Snapshot a module from an accepted static inventory. Fixed entries must be
 * distinct original input formals of this exact module; reject invalid metadata.
 * Borrow fixed array during this call only. Preserve original ordinals while
 * filtering inputs; publish output only after all four sequences succeed.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12),
 * [TPL-09](../../../../spec/language/templates.md#tpl-09).
 */
bool binding_source_interface(BindingPlan *, const StaticModule *, BindingRuntimeSurface,
    const StaticSymbol *const *, size_t, BindingInterface **);
/** Snapshot canonical intrinsic runtime fields, with empty static sequences.
 * Metadata identity is the method plus direction/ordinal, not its shape/name.
 * Publish only on success; reject an out-of-domain method without allocation.
 * Contract: [LIST-04](../../../../spec/language/lists.md#list-04).
 */
bool binding_list_interface(BindingPlan *, SemanticListMethod, BindingInterface **);
#endif
