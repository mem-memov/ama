/** @file
 * @brief Verify current named graph requests independently of scope inference and producer evaluation.
 */
#include "../../common.c"
#include "semantic/static/bindings/normalize.h"
#include "support/fixture.c"
#include "support/named.c"
#include "support/refusals.c"

/** Execute each source-to-request and failure partition explicitly. */
static void run(void) {
    it_maps_named_graph_requests_to_independent_actual_slots();
    it_retains_pending_named_requests_when_resolving_an_independent_origin();
    it_normalizes_nested_named_values_without_flattening_owners();
    it_preserves_named_residuals_without_reopening_captured_inputs();
    it_rejects_duplicate_and_unknown_named_graph_requests();
    it_unwinds_every_named_graph_request_allocation_failure();
}
/** Return success after assertions and counted owner teardown. */
int main(void) { run(); return 0; }
