/** @file
 * @brief Check actual input-slot equations against independently selected producer and standard type facts.
 */
#include <stdint.h>
#include "../../bindings/request-common.c"
#include "semantic/static/terms/application.h"
#include "semantic/static/constraints/solver.h"

/** Actual source slot identities, not the caller's reversed expected signature,
 * determine equations. A concrete standard type anchors both producer inputs and
 * the caller's repeated input; the constructor retains its own local residuals.
 */
static void it_composes_actual_binding_slots_scoped_terms_and_typed_equalities(void) {
    ModuleRoot roots[] = {{STATIC_SCOPE_ROOT_DIRECTORY, NULL, 0, MODULE_ROOT_PROJECT},
        {"../../../../../../stdlib/modules", NULL, 0, MODULE_ROOT_STANDARD}};
    setup_request_roots("anchor\n..integer\n...expected\n ... < ...type\n ... < ...type\n ... >\n", roots, 2);
    const StaticModule *caller = binding_module(project.entry);
    const StaticModule *producer = binding_module(project_find(&project, ".other"));
    const StaticModule *integer = binding_module(project_find(&project, "..integer"));
    const StaticOccurrence *site = request_head("...expected");
    const StaticSymbol *caller_symbol = static_symbol_find(caller,
        static_name_site(project.entry, "...type"), SIZE_MAX);
    BindingApplication *request;
    assert(binding_normalize_static(&binding_plan, request_interface(), site->expression, &request));
    StaticTermStore terms;
    static_term_store_init(&terms, &inventory, sources.allocator);
    const StaticTermEnvironment *caller_environment, *producer_environment;
    assert(static_term_environment(&terms, caller, NULL, &caller_environment));
    assert(static_term_environment(&terms, producer, NULL, &producer_environment));
    const StaticTerm *input, *head, *constructor, *concrete;
    assert(static_term_symbol(&terms, caller_environment, caller_symbol, &input));
    assert(static_term_symbol(&terms, caller_environment, site->symbol, &head));
    assert(static_term_nominal(&terms, producer, NULL, 0, site->site, &constructor));
    assert(static_term_nominal(&terms, integer, NULL, 0, site->site, &concrete));
    StaticTermOperand operands[] = {{.request = request->first, .value = input},
        {.request = request->first->next, .value = input}};
    const StaticTermApplication *application;
    assert(static_term_application(&terms, &binding_plan, request, caller_environment, head, operands, 2, &application));
    StaticConstraintSolver solver;
    static_constraint_init(&solver, &terms, sources.allocator);
    assert(static_constraint_equate(&solver, head, constructor, site->site));
    const StaticTerm *first_actual = NULL;
    for (const StaticTermInput *actual = application->first; actual; actual = actual->next) {
        const StaticSymbol *formal = actual->request->slot->static_symbol;
        const StaticTerm *parameter;
        assert(formal != NULL && formal->owner == producer);
        assert(static_term_symbol(&terms, producer_environment, formal, &parameter));
        assert(static_constraint_equate(&solver, parameter, actual->value, actual->request->span));
        assert(static_constraint_require(&solver, parameter, actual->request->span));
        if (first_actual == NULL) { first_actual = parameter; assert(layout_text_is(formal->name, "...a")); }
    }
    assert(static_constraint_equate(&solver, first_actual, concrete, site->site));
    assert(static_constraint_require(&solver, head, site->site));
    assert(static_constraint_require(&solver, input, site->site));
    assert(static_constraint_finish(&solver));
    const StaticTerm *value = NULL;
    StaticConstraintOutcome outcome;
    assert(static_constraint_value(&solver, input, &value, &outcome));
    assert(value == concrete && outcome == STATIC_CONSTRAINT_COMPLETE);
    assert(static_constraint_value(&solver, head, &value, &outcome));
    assert(value == constructor && value->as.nominal.residual_count == 2 && value->ground);
    assert(request->output_count == 1 && request->last->omitted && inventory.valid);
    static_constraint_destroy(&solver);
    static_term_store_destroy(&terms);
    destroy_binding();
}

/** Execute real component handoffs without claiming constructor application or code generation. */
static void run(void) { it_composes_actual_binding_slots_scoped_terms_and_typed_equalities(); }
/** Return success after assertions and counted full teardown. */
int main(void) { run(); return 0; }
