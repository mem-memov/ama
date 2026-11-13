/** @file
 * @brief Compose scalar shared dependency trees with independent contexts, no implicit roots and complete startup rollback.
 */
#include "../contexts/common.c"
#include "support/schema.c"
#include "support/values.c"
#include "support/contexts.c"
#include "support/failures.c"
/** Execute independent-context, cleanup-order and failed-startup cases. */
void run(void) {
    it_owns_nested_shared_values_per_context_without_materializing_instance_pools();
    it_publishes_no_context_or_root_when_each_shared_native_constructor_fails();
    it_cleans_every_shared_factory_allocation_prefix_and_allows_a_fresh_retry();
    it_finishes_reverse_shared_cleanup_and_preserves_the_first_destructor_error();
}
/** Return success after every explicitly registered case completes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
