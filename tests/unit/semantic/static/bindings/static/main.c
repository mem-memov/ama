/** @file
 * @brief Compose parsed static requests with actual interfaces independently of expected signature order.
 */
#include "../request-common.c"

#include "support/positions.c"
#include "support/errors.c"
#include "support/composition.c"
#include "support/omissions.c"

/** Execute every static normalization partition explicitly. */
static void run(void) {
    it_uses_actual_static_positions_without_renumbering_or_expected_signature_order();
    it_retains_unknown_static_origins_until_independent_actual_metadata_is_supplied();
    it_rejects_static_mode_order_duplicate_and_actual_arity_errors();
    it_maps_named_static_requests_to_exact_original_fields_in_arbitrary_order();
    it_uses_fixed_residual_snapshots_without_reopening_captured_formals();
    it_normalizes_nested_static_modes_without_flattening_child_requests();
    it_checks_every_static_skip_prefix_and_first_excess_position();
}

/** Return success after assertions and counted destruction. */
int main(void) { run(); return 0; }
