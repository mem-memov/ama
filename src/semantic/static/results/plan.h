/** @file
 * @brief Own static result source preparation without publishing aliases or executing producer bodies.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) generates structural output equations.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves exact source environments.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) validates required scope provenance.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns failed preparation.
 * [TPL-15](../../../../spec/language/templates.md#tpl-15) retains independent
 * scalar receivers for original declaration selections.
 */
#ifndef AMA_STATIC_RESULT_PLAN_H
#define AMA_STATIC_RESULT_PLAN_H
#include "model.h"

/** Unknown origins remain pending work; failures keep this owner destructible
 * only and never provide a guessed Type or constructor.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves unresolved evidence.
 */
typedef enum { STATIC_RESULT_OK, STATIC_RESULT_INVALID, STATIC_RESULT_CONFLICT,
    STATIC_RESULT_OUT_OF_MEMORY } StaticResultStatus;

/** Destroy before the registry, guards and every borrowed application/solver/value/source
 * owner. Solver-retained signatures must use solver-owned storage, not this
 * shorter-lived arena; equations borrow only original terms and source spans.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates lifetimes.
 */
typedef struct {
    SourceArena arena;
    StaticRegistryPlan *registry;
    StaticGuardPlan *guards;
    StaticResultGraph *first_graph;
    StaticResultGraph *last_graph;
    size_t graph_count;
    StaticResultSource *first_source;
    size_t source_count;
    StaticResultProducer *first_producer_request;
    size_t producer_request_count;
    StaticResultOperands *first_operands;
    size_t operand_count;
    StaticResultDefinition *first_definition;
    size_t definition_count;
    StaticResultContext *first_context;
    size_t context_count;
    StaticResultUse *first_use;
    size_t use_count;
    size_t dependency_count;
    StaticResultDefinitionUse *first_definition_use;
    size_t definition_use_count;
    StaticResultCache *first_cache;
    size_t cache_count;
    StaticResultStatus status;
    Diagnostic error;
} StaticResultPlan;

/** Borrow one healthy registry and its compatible guard/source/value owners without allocation.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates ownership.
 */
void static_result_init(StaticResultPlan *, StaticRegistryPlan *, SourceAllocator);
/** Release partial source/evaluation records without following borrowed edges.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds teardown.
 */
void static_result_destroy(StaticResultPlan *);
/** Bind an exact original expression to its scoped head and separate internal
 * result variable for ordinary metadata values. A named graph producer retains
 * its module origin with null head/value; it has no own Type/Template. A
 * declaration selector instead retains the exact public symbol and a separate
 * scalar receiver, disconnected from the supplier until its inputs close.
 * Preserve occurrence identity and reuse an existing key.
 * No application, definition read, body expansion or receiver equation occurs.
 * Output is untouched on refusal; partial allocations stay owned.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) binds source structure.
 */
bool static_result_source(StaticResultPlan *, const StaticTermEnvironment *,
    const AstTypeExpression *, const StaticResultSource **);
/** Bind an original explicit result or first receiver-alias definition in its
 * own environment. Parameter/runtime names are not definition identities.
 * Alias selected value stays distinct from the producer expression result.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves definition scope.
 */
bool static_result_definition(StaticResultPlan *, const StaticTermEnvironment *,
    const StaticSymbol *, const StaticResultDefinition **);
/** Register a published checked creation occurrence, retaining distinct incoming
 * paths even when owner Types are equal. Publish only after exact membership.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves required provenance.
 */
bool static_result_context(StaticResultPlan *, const StaticCreationPath *, const StaticResultContext **);
/** Evaluate one original expression with an explicit scratch stack. Pending
 * retains symbolic equations for a later evidence-driven scheduling attempt.
 * This performs no runtime execution or final alias/executable publication.
 * Output remains untouched on failure; destroy the failed owner before reuse.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates structural results from admission.
 */
bool static_result_evaluate(StaticResultPlan *, const StaticResultContext *,
    const AstTypeExpression *, StaticResultOutcome *);
/** Evaluate an original static result/alias as an independent guarded root.
 * A receiver constraint can request its existing definition without creating
 * a dependency from the producer that merely writes the receiver equality.
 * Output remains untouched on failure; no executable admission is implied.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates definitions and equalities.
 */
bool static_result_evaluate_definition(StaticResultPlan *, const StaticResultContext *,
    const StaticSymbol *, StaticResultOutcome *);
#endif
