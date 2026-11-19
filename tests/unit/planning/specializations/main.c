/** @file
 * @brief Compose closed Type evidence with actual resolved storage, lifecycle and control plans.
 * @see [T187 contract](../../../../docs/implementation/specialized-planning.md)
 */
#include "support/fixture.c"
#include "support/storage.c"
#include "support/control.c"
#include "support/lifecycle.c"
#include "support/failures.c"

/** Explicitly run each concrete planning contract without generated-C name assertions. */
static void run(void) {
    it_preserves_full_specialization_identity_and_all_storage_forms_without_static_runtime_objects();
    it_links_each_call_to_its_exact_concrete_body_and_interface();
    it_preserves_concrete_lifecycle_ownership_and_explicit_saved_position_modes();
    it_rejects_cross_capture_storage_control_lifecycle_and_scoped_cell_operands();
    it_unwinds_every_concrete_planning_allocation_and_refuses_selector_exhaustion();
}

/** Succeed only after all invariants and counted owner teardown pass. */
int main(void) { run(); return 0; }
