/** @file
 * @brief Verify exact operation-owned protection authority and complete service-state restoration independently of generated execution.
 */
#include "../common.c"
#include "runtime/objects/internal.h"
#include "support/fixture.c"
#include "support/authority.c"
#include "support/failures.c"
/** Execute every explicit protection composition scenario. */
void run(void) {
    it_borrows_only_explicit_copy_authority_and_preserves_the_outer_lease();
    it_restores_every_service_field_after_each_partial_preflight_allocation_failure();
    it_restores_protected_prefixes_when_owned_edges_are_invalid();
}
/** Return success only after every selected assertion passes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
