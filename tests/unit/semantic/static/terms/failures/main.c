/** @file
 * @brief Verify invalid identity and bounded-counter refusals before publication.
 */
#include <stdint.h>
#include "../../common.c"
#include "semantic/static/terms/store.h"

/** Foreign copied records never become scoped identities, and all query outputs
 * remain unchanged on invalid input. Saturated counters refuse without allocation.
 */
static void it_rejects_foreign_handles_and_saturated_counters_without_publication(void) {
    for (size_t operation = 0; operation < 10; operation++) {
        setup_static("@<\nargument\n...type\n@!\n", "@!\n");
        assert(static_inventory_collect(&inventory) == STATIC_OK);
        const StaticModule *module = static_module_find(&inventory, project.entry);
        const StaticModule *plain = static_module_find(&inventory, project_find(&project, ".other"));
        assert(static_name_site(project.entry, "...type").source == module->first->site.source);
        StaticTermStore store;
        static_term_store_init(&store, &inventory, sources.allocator);
        const StaticTermEnvironment *environment, *environment_output;
        const StaticTerm *base, *value;
        assert(static_term_environment(&store, module, NULL, &environment));
        assert(static_term_nominal(&store, plain, NULL, 0, (SourceSpan){0}, &base));
        StaticModule copied_module = *module;
        StaticTerm copied_value = *base;
        StaticTermEnvironment copied_environment = *environment;
        size_t before = allocation_count;
        environment_output = environment;
        value = base;
        bool query = true, success = true;
        switch (operation) {
            case 0:
                success = static_term_environment(&store, &copied_module, NULL, &environment_output);
                break;
            case 1:
                success = static_term_environment(&store, module, &copied_value, &environment_output);
                break;
            case 2:
                success = static_term_environment(&store, module, base, &environment_output);
                break;
            case 3:
                success = static_term_symbol(&store, &copied_environment, module->first, &value);
                break;
            case 4:
                success = static_term_equal(&store, base, &copied_value, &query);
                break;
            case 5:
                success = static_term_ground(&store, &copied_value, &query);
                break;
            case 6:
                store.environment_count = SIZE_MAX;
                success = static_term_environment(&store, plain, NULL, &environment_output);
                store.environment_count = 1;
                break;
            case 7:
                store.term_count = SIZE_MAX;
                success = static_term_symbol(&store, environment, module->first, &value);
                store.term_count = 1;
                break;
            case 8:
                store.term_count = SIZE_MAX;
                success = static_term_nominal(&store, module, NULL, 0, (SourceSpan){0}, &value);
                store.term_count = 1;
                break;
            case 9: {
                StaticTermCapture capture = {.formal = module->first, .value = &copied_value};
                success = static_term_nominal(&store, module, &capture, 1, (SourceSpan){0}, &value);
                break;
            }
        }
        assert(!success && allocation_count == before && query);
        assert(environment_output == environment && value == base);
        assert(store.term_count == 1 && store.environment_count == 1 && inventory.valid);
        assert(store.status == (operation >= 6 && operation <= 8 ? STATIC_TERM_OUT_OF_MEMORY : STATIC_TERM_INVALID));
        assert(diagnostic_valid(&store.error));
        static_term_store_destroy(&store);
        destroy_static();
    }
}

/** Run the independently specified foreign-domain and arithmetic partitions. */
static void run(void) { it_rejects_foreign_handles_and_saturated_counters_without_publication(); }
/** Return success after all assertions and counted owner teardown. */
int main(void) { run(); return 0; }
