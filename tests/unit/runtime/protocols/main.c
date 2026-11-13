/**
 * @file
 * @brief Compose real-resource native protocol completion, rollback and conversion checks.
 */
#include "common.c"
#include "support/construction.c"
#include "support/clone.c"
#include "support/destruction.c"
#include "support/conversion.c"
#include "support/size.c"

/** Execute explicit checked-protocol acceptance cases with per-case resource teardown. */
void run(void) {
    it_cleans_a_completed_default_payload_after_malformed_success();
    it_unwinds_a_raw_success_child_before_its_parent_receives_completion_credit();
    it_cleans_literal_construction_without_running_the_default_callback();
    it_cleans_native_source_clone_and_preserves_the_shared_original();
    it_never_destroys_a_failed_native_constructor_with_malformed_transport();
    it_unwinds_a_completed_private_clone_without_changing_its_source_root();
    it_retires_every_constant_clone_resource_before_failed_factory_return();
    it_consumes_native_credits_once_and_preserves_malformed_destroy_as_primary();
    it_keeps_condition_outputs_and_refusal_history_unchanged_after_protocol_failure();
    it_selects_size_construction_once_and_unwinds_its_completed_owned_fields();
}

/** Return success only after all protocol and counted-allocation invariants hold. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
