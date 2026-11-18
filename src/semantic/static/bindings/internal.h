/**
 * @file
 * @brief Declare transactional slot construction and allocation-free binding diagnostics.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial
 * construction. [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * keeps original error sites. [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * preserves exact slot identity and independent sequence counts.
 */
#ifndef AMA_BINDING_SEQUENCE_INTERNAL_H
#define AMA_BINDING_SEQUENCE_INTERNAL_H
#include "owner.h"

/** Allocate zeroed owned storage; stop after first failure.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void *binding_allocate(BindingPlan *, size_t);
/** Keep the first allocation refusal, allocating no diagnostic state.
 * Contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
bool binding_memory_failure(BindingPlan *);
/** Keep one borrowed original site and constant diagnostic message.
 * Contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
bool binding_invalid(BindingPlan *, SourceSpan, const char *);
/** Prepare a slot before linking; reject duplicate exact names/count overflow.
 * Prototype contains no next link and borrows only stable source/canonical data.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool binding_slot_append(BindingPlan *, BindingSequence *, BindingSlot);
/** Append only a completely prepared interface to the published owner list.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void binding_interface_publish(BindingPlan *, BindingInterface *, BindingInterface **);
/** Prepare a private application after verifying that a supplied snapshot is
 * published by this plan. A null interface retains an unresolved origin.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool binding_application_begin(BindingPlan *, const BindingInterface *, SourceSpan,
    BindingApplicationKind, BindingApplication **);
/** Validate mode/order, exact labels, omissions, storage and actual snapshot
 * positions, then append one complete request. Unknown origins stay explicit.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12),
 * [CALL-06](../../../../spec/language/calls-and-results.md#call-06).
 */
bool binding_request_append(BindingPlan *, BindingApplication *, BindingRequest);
/** Publish a fully checked application; pending is not type/closure validity.
 * Contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void binding_application_publish(BindingPlan *, BindingApplication *, BindingApplication **);
#endif
