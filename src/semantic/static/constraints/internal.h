/** @file
 * @brief Declare checked constraint scratch, diagnostic and equation publication operations.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial work.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) preserves finite evidence.
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) matches recursive signatures.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains source errors.
 */
#ifndef AMA_STATIC_CONSTRAINT_INTERNAL_H
#define AMA_STATIC_CONSTRAINT_INTERNAL_H
#include "solver.h"
#include "../terms/internal.h"
/** Allocate zeroed solver-owned storage only while healthy.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial work.
 */
void *static_constraint_allocate(StaticConstraintSolver *, size_t);
/** Record only the first error and at most two original term-definition sites;
 * supplied terms must already be checked owned handles or null.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) avoids diagnostic allocation.
 */
bool static_constraint_failure(StaticConstraintSolver *, StaticConstraintStatus, SourceSpan,
    const char *, const StaticTerm *, const StaticTerm *);
/** Queue one checked unordered pair with optional derivation parent; added is
 * false for a prior pair, and untouched on failed preparation.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) shares finite work.
 */
bool static_constraint_pair(StaticConstraintSolver *, const StaticTerm *, const StaticTerm *,
    SourceSpan, const StaticConstraintEquation *, bool *);
/** Follow acyclic solver variable links without changing value identity.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) separates solutions.
 */
const StaticTerm *static_constraint_root(const StaticConstraintSolver *, const StaticTerm *);
/** Register one exact active variable for retained declaration checks.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) preserves restrictions.
 */
StaticConstraintVariable *static_constraint_variable(StaticConstraintSolver *, const StaticTerm *);
/** Bind an unbound representative after the occurs check; changed is set only
 * on a real refinement. [TPL-10](../../../../spec/language/templates.md#tpl-10).
 */
bool static_constraint_bind(StaticConstraintSolver *, const StaticTerm *, const StaticTerm *,
    const StaticConstraintEquation *, bool *);
/** Compare declared categories/signatures; known disagreement outranks pending.
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) requires exact signatures.
 */
bool static_constraint_metadata_match(StaticConstraintSolver *, const StaticTerm *,
    const StaticTerm *, SourceSpan, bool *);
/** Validate every active alias-class declaration against its representative and
 * peers. [TPL-10](../../../../spec/language/templates.md#tpl-10) preserves prior evidence.
 */
bool static_constraint_classes(StaticConstraintSolver *, bool *);
/** Walk current value/capture links iteratively for the occurs check.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) excludes infinite terms.
 */
bool static_constraint_contains(StaticConstraintSolver *, const StaticTerm *, const StaticTerm *, bool *);
/** Check required closure without treating residual binders as unknown values.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) rejects missing anchors.
 */
bool static_constraint_closed(StaticConstraintSolver *, const StaticTerm *, bool *);
/** Process one retained pair using current representatives and original metadata.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) refines finite structure.
 */
bool static_constraint_process(StaticConstraintSolver *, const StaticConstraintEquation *, bool *, bool *);
/** Read independent resolved evidence or the original declared description.
 * This does not obtain a concrete origin from a mere expected signature.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) separates metadata from origins.
 */
void static_constraint_description(const StaticConstraintSolver *, const StaticTerm *,
    StaticCategory *, const StaticSignature **);
/** Reserve checked ordinal-indexed variable slots; arena-owned geometric growth
 * gives constant lookup and linear retained storage without changing bindings.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns old arrays.
 */
bool static_constraint_lookup_capacity(StaticConstraintSolver *, size_t);
/** Memoize exact immutable recursive signature comparisons across solve passes.
 * Deferred descriptions remain Pending until an independent new description is
 * supplied. [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
bool static_constraint_signature(StaticConstraintSolver *, const StaticSignature *,
    const StaticSignature *, SourceSpan, StaticSignatureComparison *);
/** Allocate uninitialized query-local work, marking the same sticky error while
 * allowing immediate arena teardown when that query ends.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds scratch lifetime.
 */
void *static_constraint_scratch(StaticConstraintSolver *, SourceArena *, size_t);
#endif
