/** @file
 * @brief Check internal application results never collide with source formals or omitted inputs.
 */
#include <stdint.h>
#include "../../common.c"
#include "semantic/static/terms/store.h"
#include "semantic/static/constraints/solver.h"

/** Valid projection sites permit producer-input inference, independent result
 * variables and distinct declaration nodes without preparing any runtime state.
 */
static const char *caller_source = "@<\n...owner type\n@!\nfirst\n.other\n ...result > ...alias\n"
    "second\n.other\n ... >\n";
static const char *producer_source = "@<\n...slot\n@>\n...result\n...slot\n@!\n";

/** Resolve real parsed field expression identities in source order. */
static const AstTypeExpression *field_expression(const StaticModule *module, size_t index) {
    for (const StaticOccurrence *item = module->first_occurrence; item; item = item->next) {
        if (item->root && item->receiver == NULL && item->statement && item->statement->kind == AST_DECLARATION &&
            item->context == STATIC_CONTEXT_BODY) {
            if (index == 0) { return item->expression; }
            index--;
        }
    }
    assert(false);
    return NULL;
}

/** One expression/owner key reuses a term; distinct lexical occurrences and
 * owner specializations differ. A producer input at that expression stays separate.
 */
static void it_separates_result_owner_occurrence_and_producer_input_keys(void) {
    setup_static(caller_source, producer_source);
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    const StaticModule *caller = static_module_find(&inventory, project.entry);
    const StaticModule *producer = static_module_find(&inventory, project_find(&project, ".other"));
    assert(caller->first->site.start == static_name_site(project.entry, "...owner type").start);
    StaticTermStore store;
    static_term_store_init(&store, &inventory, sources.allocator);
    const StaticTermEnvironment *environment, *applied;
    assert(static_term_environment(&store, caller, NULL, &environment));
    const AstTypeExpression *first_site = field_expression(caller, 0), *second_site = field_expression(caller, 1);
    const StaticTerm *first, *second, *again, *input, *parameter;
    assert(static_term_expression_result(&store, environment, first_site, &first));
    assert(static_term_expression_result(&store, environment, second_site, &second));
    size_t before = allocation_count;
    assert(static_term_expression_result(&store, environment, first_site, &again));
    assert(first == again && first != second && allocation_count == before);
    assert(first->category == STATIC_CATEGORY_DEFERRED && first->as.variable.symbol == NULL);
    assert(first->as.variable.role == STATIC_VARIABLE_APPLICATION);
    assert(static_term_inference(&store, environment, producer->first, NULL, first_site, &input));
    assert(first != input && input->as.variable.role == STATIC_VARIABLE_PRODUCER);
    assert(static_term_symbol(&store, environment, caller->first, &parameter));
    StaticTermCapture capture = {.formal = caller->first, .value = parameter};
    const StaticTerm *owner;
    assert(static_term_nominal(&store, caller, &capture, 1, first_site->span, &owner));
    assert(static_term_environment(&store, caller, owner, &applied));
    assert(static_term_expression_result(&store, applied, first_site, &again) && again != first);
    assert(caller->parameter_count == 1 && producer->parameter_count == 1);
    StaticConstraintSolver solver;
    static_constraint_init(&solver, &store, sources.allocator);
    assert(static_constraint_require(&solver, first, first_site->span));
    assert(!static_constraint_finish(&solver) && solver.status == STATIC_CONSTRAINT_UNRESOLVED);
    assert(diagnostic_valid(&solver.error) && solver.error.related_count == 2);
    assert(solver.error.related[0].location.span.source == first_site->span.source);
    assert(solver.error.related[0].location.span.start == first_site->span.start);
    assert(strcmp(solver.error.related[0].name, ".main") == 0);
    static_constraint_destroy(&solver);
    static_term_store_destroy(&store);
    destroy_static();
}

/** Refuse each actual allocation and foreign copied syntax; no internal result
 * is published and accepted source inventories remain untouched.
 */
static void it_unwinds_expression_result_allocation_and_foreign_site_refusals(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_static(caller_source, producer_source);
        assert(static_inventory_collect(&inventory) == STATIC_OK);
        const StaticModule *caller = static_module_find(&inventory, project.entry);
        StaticTermStore store;
        static_term_store_init(&store, &inventory, sources.allocator);
        const StaticTermEnvironment *environment;
        assert(static_term_environment(&store, caller, NULL, &environment));
        const AstTypeExpression *site = field_expression(caller, 0);
        const StaticTerm *output = NULL;
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool success = static_term_expression_result(&store, environment, site, &output);
        if (!failure) { assert(success); total = allocation_count - before; assert(total); }
        else { assert(!success && output == NULL && store.term_count == 0 && store.status == STATIC_TERM_OUT_OF_MEMORY); }
        failure_at = 0;
        static_term_store_destroy(&store);
        destroy_static();
    }
    for (size_t invalid = 0; invalid < 3; invalid++) {
        setup_static(caller_source, producer_source);
        assert(static_inventory_collect(&inventory) == STATIC_OK);
        const StaticModule *caller = static_module_find(&inventory, project.entry);
        StaticTermStore store;
        static_term_store_init(&store, &inventory, sources.allocator);
        const StaticTermEnvironment *environment;
        assert(static_term_environment(&store, caller, NULL, &environment));
        AstTypeExpression copied = *field_expression(caller, 0);
        StaticTermEnvironment copied_environment = *environment;
        const StaticTerm *output = NULL;
        assert(!static_term_expression_result(&store, invalid == 1 ? &copied_environment : environment,
            invalid == 2 ? NULL : &copied, &output));
        assert(output == NULL && store.status == STATIC_TERM_INVALID && inventory.valid);
        static_term_store_destroy(&store);
        destroy_static();
    }
}

/** Run exact identity, diagnostic and preparation-failure partitions. */
static void run(void) {
    it_separates_result_owner_occurrence_and_producer_input_keys();
    it_unwinds_expression_result_allocation_and_foreign_site_refusals();
}
/** Return success after assertions and full counted teardown. */
int main(void) { run(); return 0; }
