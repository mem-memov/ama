/** @file
 * @brief Run parser/project/static-inventory acceptance and resource partitions.
 */
#include "../common.c"
#include <stdint.h>

/** Require one exact visible entry-module static identity. */
static const StaticSymbol *named(const char *name) {
    const StaticModule *module = static_module_find(&inventory, project.entry);
    const StaticSymbol *symbol = static_symbol_find(module, static_name_site(project.entry, name), SIZE_MAX);
    assert(symbol != NULL);
    return symbol;
}

#include "support/scope.c"
#include "support/failures.c"

/** Execute actual compositions rather than treating private primitive tests as
 * acceptance for the whole collector. */
static void run(void) {
    it_binds_ordinary_input_and_result_type_lines_without_static_runtime_fields();
    it_binds_closed_graph_pairs_in_public_order_without_runtime_storage();
    it_rejects_ordinary_body_only_placeholder_types_at_original_sources();
    it_rejects_static_application_of_a_known_concrete_type();
    it_unwinds_every_whole_static_collection_allocation_failure();
}

/** Return success after explicit assertions and counted ownership teardown. */
int main(void) { run(); return 0; }
