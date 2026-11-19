/** @file
 * @brief Compile concrete native operations through ordinary Ama template substitution.
 * @see [T190 contract](../../../../docs/implementation/native-template-composition.md)
 */
#include "../execution/common.c"
#include "support/capabilities.c"
#include "support/exports.c"
#include "support/refusals.c"
#include "support/alignment.c"

/** Run each independently specified concrete-native composition partition. */
static void run(void) {
    it_executes_native_owned_reference_and_list_fields_from_closed_static_exports();
    it_executes_all_required_native_capabilities_after_template_substitution();
    it_refuses_missing_native_capabilities_without_generating_specialized_companions();
    it_reports_native_literal_failure_before_entering_a_specialized_body();
    it_refuses_open_static_suppliers_for_every_native_storage_form();
    it_refuses_invalid_native_entry_types_and_preserves_authored_and_previous_outputs();
    it_checks_selected_native_lifecycle_statuses_through_template_execution();
    it_preserves_actual_overaligned_native_layout_after_substitution();
}

/** Return success only after generated execution and counted frontend teardown. */
int main(void) { run(); return 0; }
