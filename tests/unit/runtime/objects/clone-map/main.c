/** @file
 * @brief Exhaust exact scalar-tree reference maps, ordinary service projection and cleanup of failed independent copies.
 */
#include "../common.c"
#include "runtime/program/program.h"
#include "../../program-construction/support/schema.c"
#include "support/fixture.c"
#include "support/projection.c"
#include "support/matrix.c"
#include "support/failures.c"
/** Execute finite mapping and failed-prefix scenarios. */
void run(void) {
    it_maps_all_scalar_reference_substitutions_by_complete_identity();
    it_preserves_source_and_external_state_when_each_native_clone_fails();
    it_unwinds_every_scalar_clone_allocation_without_publishing_or_reinitializing();
}
/** Return success only after every explicit scenario passes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
