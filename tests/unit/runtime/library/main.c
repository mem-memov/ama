/** @file
 * @brief Exercise actual context/object lifecycle behind checked component controls.
 */
#include "common.c"
#include "support/identity.c"
#include "support/lifetime.c"
#include "support/failures.c"
#include "support/allocation.c"
#include "support/admission.c"
#include "support/code.c"
/** Register explicit publication, isolation, admission, continuation-state and failure checks. */
void run(void) {
    it_tracks_code_pins_and_snapshot_owners_without_counter_wrap();
    it_retains_native_code_through_independent_move_only_leases();
    it_keeps_kernel_references_and_component_controls_distinct();
    it_rejects_cross_context_foreign_and_obsolete_controls();
    it_constructs_only_explicit_roots_and_disposes_yielded_state_without_resume();
    it_refuses_busy_and_invalid_factory_requests_without_effects();
    it_checks_destination_shapes_before_lifecycle_effects();
    it_returns_startup_failure_before_context_publication();
    it_fails_only_the_admitted_context_and_unwinds_all_live_roots();
    it_consumes_admitted_destructor_failure_exactly_once();
    it_unwinds_every_lifecycle_allocation_failure();
    it_never_wraps_instance_control_generation();
    it_preserves_admission_provenance_during_native_callbacks();
    it_rejects_a_simultaneous_context_executor();
    it_preserves_context_when_retirement_is_not_admitted();
}
/** Return only after address-sensitive resources and all counted ownership are released. */
int main(void) { run(); return EXIT_SUCCESS; }
