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
#include "support/signatures.c"
#include "support/failures.c"

/** Execute actual compositions rather than treating private primitive tests as
 * acceptance for the whole collector. */
static void run(void) {
    it_binds_output_only_type_heads_and_retains_actual_alias_obligations();
    it_binds_result_definitions_and_foreign_names_without_runtime_storage();
    it_rejects_body_only_names_and_sources_before_alias_reception();
    it_preserves_aliases_received_inside_runtime_blocks_for_later_uses();
    it_checks_explicit_and_repeated_inferred_recursive_signature_agreement();
    it_rejects_infinite_known_signatures_but_accepts_shared_acyclic_metadata();
    it_retains_applied_and_projected_metadata_as_explicit_pending_obligations();
    it_allows_partial_known_constructor_inputs_when_outputs_request_closure();
    it_checks_concrete_constructor_patterns_without_importing_foreign_formals();
    it_collects_deep_real_signature_definitions_without_recursive_c_activation();
    it_unwinds_every_whole_static_collection_allocation_failure();
}

/** Return success after explicit assertions and counted ownership teardown. */
int main(void) { run(); return 0; }
