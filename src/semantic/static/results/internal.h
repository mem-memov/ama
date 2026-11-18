/** @file
 * @brief Share checked source-owner allocation and first-error operations for static result preparation.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original diagnostic storage.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) checks actual scope paths.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) scopes source/definition uses.
 */
#ifndef AMA_STATIC_RESULT_INTERNAL_H
#define AMA_STATIC_RESULT_INTERNAL_H
#include "plan.h"
#include "../guards/internal.h"

/** Allocate zeroed result-plan records while healthy; failures allocate no report.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial construction.
 */
void *static_result_allocate(StaticResultPlan *, size_t);
/** Preserve a first constant message and actual original source span.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves locations.
 */
bool static_result_fail(StaticResultPlan *, StaticResultStatus, SourceSpan, const char *);
/** Propagate guard/application/solver/value errors without copying borrowed data.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves first errors.
 */
bool static_result_dependencies(StaticResultPlan *);
/** Exact context membership precedes following its borrowed creation handle.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves checked paths.
 */
bool static_result_has_context(const StaticResultPlan *, const StaticResultContext *);
/** Bind one expression/creation/definition use with stable repeated identity.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves source operators.
 */
bool static_result_use(StaticResultPlan *, const StaticResultContext *, const AstTypeExpression *,
    const StaticDefinitionPath *, StaticResultUse **);
/** Enter the definition guard before interning its evaluation occurrence;
 * cached values cannot bypass incoming path checks.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) guards definition reads.
 */
bool static_result_definition_use(StaticResultPlan *, const StaticResultContext *, const StaticSymbol *,
    const StaticDefinitionPath *, SourceSpan, StaticResultDefinitionUse **);
/** Reuse a checked complete definition value only after incoming guards refresh.
 * A hit requires an actual-context proof or fully closed actual ancestor keys;
 * otherwise a successful miss retains valid value equations and requests RHS
 * replay in the actual context. Never replace the current scoped identity.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves immutable results.
 */
bool static_result_cache_read(StaticResultPlan *, StaticResultDefinitionUse *, bool *);
/** Publish only checked RHS work with independently closed owner, expression
 * and selected value. Unknown unused inputs cannot be hidden by a constant result.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) requires complete producers.
 */
bool static_result_cache_store(StaticResultPlan *, StaticResultDefinitionUse *, bool *);
/** Exact live use membership precedes cache payload access.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves scoped identity.
 */
bool static_result_has_definition_use(const StaticResultPlan *, const StaticResultDefinitionUse *);
/** Prepare one expression's original operands without evaluating descendants.
 * Child values retain the common lexical context and definition ancestry.
 * Repeated preparation allocates nothing; output actions remain independent.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves structural substitution.
 */
bool static_result_prepare(StaticResultPlan *, StaticResultUse *);
/** Exact membership protects expression operations from copied/foreign handles.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) scopes work identities.
 */
bool static_result_has_use(const StaticResultPlan *, const StaticResultUse *);
/** Retain an exact visited expression or guarded definition dependency once.
 * Only one child alternative is nonnull; no receiver equality becomes an edge.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves checked provenance.
 */
bool static_result_dependency(StaticResultPlan *, StaticResultUse *, const StaticResultUse *,
    const StaticResultDefinitionUse *);
/** Apply prepared operands through the actual-origin adapter and enter its
 * creation path before exposing a producer context. This does not evaluate
 * child definitions or static outputs. Pending origins retain the same record
 * until independent origin evidence arrives; repeated checks create no frames.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) guards producer expansion.
 */
bool static_result_apply(StaticResultPlan *, StaticResultUse *, StaticApplicationOutcome *);
/** Prepare immutable original operands for a registered source/environment.
 * No application state or readiness is returned. Caller output is untouched on
 * refusal; every actual use retains separate child and expansion provenance.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) fixes lexical inputs;
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) forbids metadata-only admission.
 */
bool static_result_operands(StaticResultPlan *, const StaticResultSource *,
    const StaticTermApplication **);
/** Apply original source/operand equations to one retained application state,
 * without entering a producer or evaluating derived names/outputs. Callers have
 * validated source/prepared/state ownership; actual uses still run creation guards.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) keeps structural recovery exact.
 */
bool static_result_application(StaticResultPlan *, const StaticResultSource *,
    const StaticTermApplication *, const StaticApplicationRecord **);
#endif
