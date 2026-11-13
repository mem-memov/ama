/** @file
 * @brief Verify continuation typing and positive copy-boundary proofs without guessing runtime saved positions or aliases.
 */
#include "../typed-common.c"
#include "support/forms.c"
#include "support/origins.c"
#include "support/failures.c"
/** Run every explicit transfer, origin proof, effect barrier and partial-allocation case. */
void run(void) {
    it_keeps_ordinary_handle_and_continuation_transfer_kinds_distinct();
    it_accepts_owned_scalar_continuation_sources_from_owned_and_reference_storage();
    it_rejects_resume_inputs_whole_list_sources_and_invalid_prefix_forms();
    it_rejects_positive_direct_reference_aliases_and_current_root_overlap();
    it_does_not_infer_aliases_across_blocks_calls_yields_or_unknown_reference_stores();
    it_keeps_external_references_outside_clone_capability_and_ownership_boundaries();
    it_unwinds_every_positive_reference_origin_allocation();
}
/** Return success only after all named assertion cases complete. */
int main(void) { run(); return EXIT_SUCCESS; }
