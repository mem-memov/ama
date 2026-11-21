/** @file
 * @brief Share checked source-owner allocation and first-error operations for static result preparation.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves source and omitted-input identities.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original diagnostic storage.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) checks actual scope paths.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) scopes source/definition uses.
 * [TPL-15](../../../../spec/language/templates.md#tpl-15) shares guarded scalar
 * equations between annotation facts and actual declaration uses.
 */
#ifndef AMA_STATIC_RESULT_INTERNAL_H
#define AMA_STATIC_RESULT_INTERNAL_H
#include "plan.h"
#include "../guards/internal.h"

/** Add a simultaneous equation from an original selected structural definition.
 * Actual source/definition identities belong to the registered result owner;
 * null definitions and nonselections add no equation. Full supplier closure is
 * a separate admission requirement under
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) and
 * [TPL-15](../../../../spec/language/templates.md#tpl-15).
 */
bool static_result_selection_equation(StaticResultPlan *, const StaticResultSource *,
    const StaticResultDefinitionUse *);

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
/** Check exact original source-record membership without dereferencing a copy.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves lexical source authority.
 */
bool static_result_has_source(const StaticResultPlan *, const StaticResultSource *);
/** Prepare one named producer use, validating actual foreign labels and retaining
 * every input in original formal order. Missing inputs have deterministic lexical
 * holes. No guard entry, body evaluation or producer Type is created. Source and
 * receiver must be owned/existing; output is unchanged on any refusal.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) requires complete producer inputs later.
 */
bool static_result_prepare_producer(StaticResultPlan *, const StaticResultSource *, const StaticResultProducer **);
/** Enter a prepared actual named producer before following its definitions.
 * Caller validated the use and receiver and prepared its full input map.
 * A repeated unknown origin remains Pending with no speculative child context.
 * Existing contexts refresh actual graph/key checks on every retry.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) checks ancestry before expansion.
 */
bool static_result_apply_producer(StaticResultPlan *, StaticResultUse *, StaticApplicationOutcome *);
/** Require all public definitions and root producer statements independently of
 * a current value-read path. Exact actual-context requests are idempotent; a
 * closed-key recurrence links to an already required actual ancestor witness.
 * Independent paths retain their own replay opportunity, never a guessed root.
 * Ordinary contexts have no static graph and succeed with null output. On failure
 * the output remains unchanged; partial source/guard work stays owned.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) closes known identity cycles.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) checks every producer definition.
 */
bool static_result_require_graph(StaticResultPlan *, const StaticResultContext *, StaticResultGraph **);
/** Advance every independent root of one actual graph witness once. This does
 * not recursively wait for dependent graph certificates. Local source values
 * and actual guards must pass before recording the canonical local certificate;
 * ordinary body/admission obligations remain in the registry dependency graph.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) permits finite validation groups.
 */
bool static_result_graph_pass(StaticResultPlan *, StaticResultGraph *, StaticResultOutcome *);
#endif
