/** @file
 * @brief Compile a separate source library through explicit roots while retaining canonical code identity and private instance state.
 * @see [T191 contract](../../../../docs/implementation/template-source-delivery.md)
 */
#include "../execution/common.c"
#include "../../compiler/source-delivery/common.c"
#include "support/reuse.c"
#include "support/dependencies.c"

/** Run actual separately rooted source delivery, not a binary template ABI. */
static void run(void) {
    it_reuses_source_library_types_and_code_across_root_orders_without_sharing_instances();
    it_reports_imported_native_literal_failure_before_entering_the_library_template();
    it_rebuilds_changed_external_native_headers_in_actual_consumers();
    it_refuses_missing_imported_native_and_external_c_inputs_without_publication();
}

/** Return success only after actual compiled effects and counted owner teardown. */
int main(void) { run(); return 0; }
