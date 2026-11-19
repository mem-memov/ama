/** @file
 * @brief Verify lexical static identity without specialization or equation solving.
 */
#include <stdint.h>
#include "../../common.c"
#include "semantic/static/terms/store.h"

/** Same-spelled original symbols differ by environment; repeated lookups reuse
 * records and do not allocate. All owners tear down before borrowed sources.
 */
static void it_preserves_scoped_symbols_and_reuses_exact_keys(void) {
    setup_static("@<\nargument\n...type\n@!\nfirst\n...type\nsecond\n...type\n", "@<\nargument\n...type\n@!\n");
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    const StaticModule *main_module = static_module_find(&inventory, project.entry);
    const StaticModule *other_module = static_module_find(&inventory, project_find(&project, ".other"));
    assert(main_module->first->site.start == static_name_site(project.entry, "...type").start);
    StaticTermStore store;
    static_term_store_init(&store, &inventory, sources.allocator);
    const StaticTermEnvironment *main_environment, *other_environment, *again;
    assert(static_term_environment(&store, main_module, NULL, &main_environment));
    assert(static_term_environment(&store, other_module, NULL, &other_environment));
    size_t before = allocation_count;
    assert(static_term_environment(&store, main_module, NULL, &again));
    assert(again == main_environment && allocation_count == before);
    const StaticTerm *first, *second, *repeated;
    assert(static_term_symbol(&store, main_environment, main_module->first, &first));
    assert(static_term_symbol(&store, other_environment, other_module->first, &second));
    before = allocation_count;
    assert(static_term_symbol(&store, main_environment, main_module->first, &repeated));
    assert(first == repeated && first != second && allocation_count == before);
    assert(first->as.variable.role == STATIC_VARIABLE_FORMAL);
    const AstNode *field = NULL, *next_field = NULL;
    for (const StaticOccurrence *item = main_module->first_occurrence; item; item = item->next) {
        if (item->statement && item->statement->kind == AST_DECLARATION) {
            if (!field) { field = item->statement; }
            else if (field != item->statement) { next_field = item->statement; }
        }
    }
    assert(field && next_field);
    assert(static_term_inference(&store, main_environment, main_module->first, field, NULL, &first));
    assert(static_term_inference(&store, main_environment, main_module->first, next_field, NULL, &second));
    assert(static_term_inference(&store, main_environment, main_module->first, field, NULL, &repeated));
    assert(first == repeated && first != second && first->as.variable.role == STATIC_VARIABLE_FIELD);
    StaticSymbol copied = *main_module->first;
    const StaticTerm *sentinel = first;
    assert(!static_term_symbol(&store, main_environment, &copied, &sentinel));
    assert(sentinel == first && store.status == STATIC_TERM_INVALID && inventory.valid);
    assert(!static_term_symbol(&store, main_environment, main_module->first, &sentinel));
    static_term_store_destroy(&store);
    destroy_static();
}

/** Producer sites and field sites are distinct keys; copied syntax is refused.
 * Deferred aliases retain their unresolved category instead of becoming Type.
 */
static void it_preserves_producer_roles_and_deferred_alias_metadata(void) {
    setup_static("@...\n.other\n ...out > ...alias\n", "@...\n...out\n...input\n");
    assert(static_inventory_collect(&inventory) == STATIC_OK);
    const StaticModule *module = static_module_find(&inventory, project.entry);
    StaticTermStore store;
    static_term_store_init(&store, &inventory, sources.allocator);
    const StaticTermEnvironment *environment;
    assert(static_term_environment(&store, module, NULL, &environment));
    const StaticOccurrence *site = module->first_occurrence;
    while (site && site->origin == NULL) { site = site->next; }
    assert(site && site->expression);
    const StaticModule *origin = static_module_find(&inventory, project_find(&project, ".other"));
    const StaticSymbol *formal = static_symbol_find(origin, static_name_site(origin->definition, "...input"));
    const StaticTerm *producer, *again, *alias;
    assert(static_term_inference(&store, environment, formal, NULL, site->expression, &producer));
    assert(static_term_inference(&store, environment, formal, NULL, site->expression, &again));
    assert(producer == again && producer->as.variable.role == STATIC_VARIABLE_PRODUCER);
    const StaticSymbol *symbol = static_symbol_find(module, static_name_site(project.entry, "...alias"));
    assert(symbol && symbol->role == STATIC_ALIAS);
    assert(static_term_symbol(&store, environment, symbol, &alias));
    assert(alias->category == STATIC_CATEGORY_DEFERRED && alias->as.variable.role == STATIC_VARIABLE_DERIVED);
    AstTypeExpression copied = *site->expression;
    assert(!static_term_inference(&store, environment, formal, NULL, &copied, &again));
    assert(again == producer && store.status == STATIC_TERM_INVALID);
    static_term_store_destroy(&store);
    destroy_static();
}

/** Allocation refusal covers environment and variable publication independently;
 * repeated successful keys require no allocations, even with a refusal armed.
 */
static void it_unwinds_environment_and_variable_allocation_failures(void) {
    for (size_t operation = 0; operation < 2; operation++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_static("@<\nargument\n...type\n@!\n", "@!\n");
            assert(static_inventory_collect(&inventory) == STATIC_OK);
            const StaticModule *module = static_module_find(&inventory, project.entry);
            StaticTermStore store;
            static_term_store_init(&store, &inventory, sources.allocator);
            const StaticTermEnvironment *environment = NULL;
            const StaticTerm *value = NULL;
            if (operation) { assert(static_term_environment(&store, module, NULL, &environment)); }
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            bool success = operation ? static_term_symbol(&store, environment, module->first, &value) :
                static_term_environment(&store, module, NULL, &environment);
            if (!failure) {
                assert(success);
                total = allocation_count - before;
                assert(total);
                failure_at = allocation_count + 1;
                assert(operation ? static_term_symbol(&store, environment, module->first, &value) :
                    static_term_environment(&store, module, NULL, &environment));
            } else {
                assert(!success && store.status == STATIC_TERM_OUT_OF_MEMORY);
                assert(store.term_count == 0 && value == NULL);
                assert(store.environment_count == operation);
            }
            failure_at = 0;
            static_term_store_destroy(&store);
            destroy_static();
        }
    }
}

/** Explicit runner keeps each component scenario independently executable. */
static void run(void) {
    it_preserves_scoped_symbols_and_reuses_exact_keys();
    it_preserves_producer_roles_and_deferred_alias_metadata();
    it_unwinds_environment_and_variable_allocation_failures();
}
/** Return success after all assertions and counted owner teardown. */
int main(void) { run(); return 0; }
