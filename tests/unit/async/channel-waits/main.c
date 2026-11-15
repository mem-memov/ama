/** @file
 * @brief Verify native channel wakeup, exact registration cancellation and independent post-context lifetime.
 * Compose real channel and event components without an Ama executor or mock callback queue.
 */
#include "common.c"
#include "support/readiness.c"
#include "support/cancellation.c"
#include "support/shutdown.c"
#include "support/failures.c"
#include "support/concurrency.c"

/** Run readiness/provider admission cases under exact native allocation/code ownership. */
void run(void) {
    prepare_identity_history();
    it_retains_receive_readiness_on_both_sides_of_synchronous_registration();
    it_wakes_capacity_only_after_the_exact_requested_length_fits();
    it_rejects_unarmed_sources_and_acks_completed_retired_and_closed_sources();
    it_cancels_exact_keys_and_preserves_a_replacement_on_the_same_event_slot();
    it_withdraws_publication_without_discarding_channel_data();
    it_wakes_both_directions_on_their_terminal_endpoint_transitions();
    it_retires_closed_event_sources_and_channel_storage_after_both_ports_disappear();
    it_signals_oversize_and_already_completed_requests_without_redundant_owners();
    it_preserves_wait_outputs_on_reference_exhaustion_and_invalid_source_shapes();
    it_never_loses_receive_readiness_when_registration_races_publication();
    it_wakes_a_native_host_wait_when_the_other_endpoint_closes();
    it_retires_delivery_when_cancellation_races_an_already_selected_publication();
}
/** Success requires all live native ownership to retire, excluding declared image identity history. */
int main(void) { run(); no_resources(); return EXIT_SUCCESS; }
