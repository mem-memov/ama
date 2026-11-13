/** @file
 * @brief Compose real declaration recipes with final self identities, private construction and complete unwind.
 */
#include "../objects/common.c"
#include "runtime/program/program.h"
#include "support/schema.c"
#include "support/observers.c"
#include "support/success.c"
#include "support/failures.c"
/** Run ordered construction, self visibility and invalidation scenarios. */
void run(void) {
    it_constructs_real_self_initializers_once_at_final_private_addresses();
    it_starts_the_program_with_the_same_typed_self_recipe_path();
    it_never_revives_self_handles_observed_before_any_failed_constructor();
    it_releases_every_allocation_prefix_of_self_initialization();
    it_forms_self_without_acquiring_running_state_and_rejects_invalid_identity();
}
/** Return success only after each explicit case completes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
