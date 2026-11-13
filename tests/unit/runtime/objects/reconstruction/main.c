/** @file
 * @brief Verify final self identity and failure boundaries of default/literal staged reconstruction.
 */
#include "../common.c"
#include "runtime/program/program.h"
#include "runtime/objects/internal.h"
#include "../../program-construction/support/schema.c"
#include "support/fixture.c"
#include "support/success.c"
#include "support/failures.c"
/** Run successful bank swaps and every preparation/destruction failure boundary. */
void run(void) {
    it_keeps_final_self_identity_and_native_addresses_through_default_and_literal_commit();
    it_preserves_the_old_tree_when_each_prepared_native_constructor_fails();
    it_preserves_the_old_tree_at_every_reconstruction_allocation_failure();
    it_finishes_old_and_prepared_cleanup_when_each_destructive_commit_hook_fails();
}
/** Return success after every explicit assertion completes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
