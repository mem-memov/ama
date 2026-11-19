/** @file
 * @brief Share exact registry membership, key interning and first-error ownership boundaries.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) preserves canonical keys.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) retains source paths.
 */
#ifndef AMA_STATIC_REGISTRY_INTERNAL_H
#define AMA_STATIC_REGISTRY_INTERNAL_H
#include "plan.h"
#include "../guards/internal.h"

/** Allocate zeroed owned metadata while healthy; refusal is sticky.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
void *static_registry_allocate(StaticRegistryPlan *, size_t);
/** Preserve the first diagnostic without allocating reporting storage.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03).
 */
bool static_registry_fail(StaticRegistryPlan *, StaticRegistryStatus, SourceSpan, const char *);
/** Propagate a borrowed guard/application/solver failure without replacing it.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves actual failure provenance.
 */
bool static_registry_dependencies(StaticRegistryPlan *);
/** Reuse or register one normalized closed owner identity; no body is checked.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05).
 */
bool static_registry_candidate(StaticRegistryPlan *, const StaticTermEnvironment *, SourceSpan,
    StaticRegistryCandidate **);
/** Associate one already owned use after a successful guard refresh. Previously
 * closed keys are immutable, so their association needs no duplicate query.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) retains unknown uses.
 */
bool static_registry_bind(StaticRegistryPlan *, StaticRegistryUse *);
/** Check exact owned use membership before following caller pointers.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves actual incoming uses.
 */
bool static_registry_has_use(const StaticRegistryPlan *, const StaticRegistryUse *);
/** Invalidate provisional graph certificates after adding an obligation.
 * Local completion remains independent evidence.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11).
 */
void static_registry_invalidate_checks(StaticRegistryPlan *);
/** Retain exactly one already validated guard path without recursive ancestry.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) owns incoming uses.
 */
bool static_registry_retain(StaticRegistryPlan *, const StaticCreationPath *, StaticRegistryUse **);
/** Compute nonallocating greatest-fixed-point closure on refreshed healthy records.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) checks all reachable obligations.
 */
void static_registry_close_checks(StaticRegistryPlan *, StaticRegistryKeyState, StaticRegistryCheckState *);
#endif
