/** @file
 * @brief Share checked constant metadata allocation and first-error propagation.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains original diagnostics.
 */
#ifndef AMA_STATIC_CONSTANT_INTERNAL_H
#define AMA_STATIC_CONSTANT_INTERNAL_H
#include "plan.h"
#include "../publication/internal.h"

/** Allocate zeroed metadata while healthy, retaining partial owner state.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds construction.
 */
void *static_constant_allocate(StaticConstantPlan *, size_t);
/** Record only the first constant error with original source provenance.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves first errors.
 */
bool static_constant_fail(StaticConstantPlan *, StaticConstantStatus, SourceSpan, const char *);
/** Preserve borrowed publication/result errors without copying source storage.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves diagnostic lifetime.
 */
bool static_constant_dependencies(StaticConstantPlan *);
/** Snapshot only the actual concrete Type's own shared fields; reject duplicate
 * own names/reference storage before first-match normalization can occur.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) fixes own member identity.
 */
bool static_constant_surface(StaticConstantPlan *, const StaticTerm *, BindingInterface **);
/** Prepare selected declaration/type obligations from already normalized own
 * requests. Keep the enclosing selection private until every member succeeds.
 * [CONST-06](../../../../spec/language/declarations.md#const-06) retains symbolic declared Types.
 */
bool static_constant_members(StaticConstantPlan *, StaticConstantSelection *);
/** Validate an actual registered scope against an independently determined
 * canonical namespace Type; never add equations or infer from a scope candidate.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) separates inference and lookup.
 */
bool static_constant_scope(StaticConstantPlan *, const StaticResultContext *, const StaticTerm *);
#endif
