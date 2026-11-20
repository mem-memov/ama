/** @file
 * @brief Run composed library configuration, typed-schema, identity and dependency checks.
 */
#include "common.c"
#include "support/contracts.c"
#include "support/templates.c"
#include "support/inputs.c"
#include "support/failures.c"
#include "support/identities.c"
#include "support/native-profiles.c"

/** Run complete typed, negative, dependency and cleanup scenarios explicitly. */
void run(void) {
    it_rejects_open_static_and_synthetic_template_export_roots();
    it_rejects_rebinding_a_completed_library_model();
    it_requires_native_profile_capabilities_and_failure_shape();
    it_names_complete_abi_contracts_and_rejects_identity_collisions();
    it_tracks_link_inputs_and_preserves_native_companions();
    it_resolves_explicit_exports_and_complete_owned_schemas();
    it_rejects_missing_and_incompatible_explicit_contracts();
    it_tracks_dependency_changes_and_preserves_handwritten_inputs();
    it_unwinds_every_library_resolution_allocation();
}
/** Return after the actual configuration/frontend/model boundaries pass. */
int main(void) { run(); return EXIT_SUCCESS; }
