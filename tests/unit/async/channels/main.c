/** @file
 * @brief Verify actual bounded channel ownership, refusal, cleanup and native concurrency.
 * Scenarios use public creator-dispatch endpoints and the real typed message codec.
 */
#include "common.c"
#include "support/capacity.c"
#include "support/lifetime.c"
#include "support/rejection.c"
#include "support/failures.c"
#include "support/concurrency.c"

/** Run independently testable ownership and delivery scenarios. */
void run(void) {
    it_transfers_whole_fifo_messages_and_releases_capacity_before_processing();
    it_enforces_exact_byte_capacity_and_individual_message_size();
    it_reuses_ring_slots_without_moving_or_merging_received_owners();
    it_refunds_exact_variable_message_lengths_at_take();
    it_drains_after_sender_disposal_and_retains_taken_data_after_service_retirement();
    it_aborts_delivery_and_preserves_rejected_sender_data();
    it_copies_schema_and_preserves_rejected_inputs_and_receiver_outputs();
    it_keeps_independent_channel_order_and_ownership_separate();
    it_cleans_every_creation_prefix_and_rejects_invalid_capacities_and_outputs();
    it_transfers_fifo_messages_between_actual_native_producer_and_consumer_threads();
    it_linearizes_abort_against_send_without_losing_or_double_releasing_an_owner();
}
/** Success requires every native allocation and creator-code credit to retire. */
int main(void) { run(); no_resources(); return EXIT_SUCCESS; }
