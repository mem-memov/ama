/** @file
 * @brief Run real exclusive-access and complete-plan revalidation components without publication.
 */
#include "common.c"
#include "support/leases.c"
#include "support/scope.c"
#include "support/io.c"
/** Each phase test composes the production source and filesystem owners. */
void run(void) {
    it_serializes_independent_leases_and_releases_descriptors();
    it_revalidates_complete_scope_and_original_bytes();
    it_unwinds_every_lease_and_revalidation_allocation();
    it_refuses_every_measured_lease_io_checkpoint();
}
/** No source change is needed to test lease and input ownership. */
int main(void) { run(); return EXIT_SUCCESS; }
