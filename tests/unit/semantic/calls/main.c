/** @file
 * @brief Verify ordinary entry/resume/retained/constant binding types, omissions, caller context and exact diagnostics.
 */
#include "../typed-common.c"
#include "support/matrix.c"
#include "support/defaults.c"
#include "support/diagnostics.c"
#include "support/failures.c"
/** Run all storage, interface, omission, diagnostic and ownership cases explicitly. */
void run(void) {
    it_matches_all_storage_forms_for_inputs_and_each_output_mode();
    it_preserves_independent_positions_defaults_skips_and_literal_bytes();
    it_resolves_sources_and_receivers_in_the_caller_not_the_field_owner();
    it_rejects_binding_role_suffix_arity_and_copy_capability_errors();
    it_retains_the_declaration_and_both_binding_sites_for_duplicate_labels();
    it_rejects_invalid_modes_and_mixed_styles_before_semantic_analysis();
    it_unwinds_each_allocation_in_complete_ordinary_binding_resolution();
}
/** Return success only after the leaf's explicit assertions complete. */
int main(void) { run(); return EXIT_SUCCESS; }
