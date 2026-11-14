/** @file
 * @brief Compile exact literal input and sequential result-receiver contracts with actual native constructors and destruction effects.
 */
#include "../execution/common.c"
#include "support/bytes.c"
#include "support/receivers.c"

/** Exercise raw bytes, constructor failure, receiver inference/placeholders and partial output progress independently. */
void run(void) {
    it_preserves_literal_bytes_and_selects_one_constructor_with_lf_crlf_and_empty_payloads();
    it_keeps_literal_constructor_failure_before_body_entry_and_result_publication();
    it_executes_inferred_owned_reference_receivers_and_output_placeholders();
    it_resolves_each_output_receiver_after_the_previous_transfer_commits();
    it_retains_an_earlier_output_when_a_later_receiver_conversion_fails();
}

/** Return success only after all explicit compiled cases complete. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
