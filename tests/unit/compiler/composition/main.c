/** @file
 * @brief Verify complete public frontend-to-plan composition preserves typed identities, source spans and every control successor.
 */
#include <unistd.h>
#include "../common.c"
#include "project/inputs/internal.h"
#include "support/fixture.c"
#include "support/compare.c"
#include "support/projection/storage.c"
#include "support/projection/control.c"
#include "support/types.c"
#include "support/projects.c"
#include "support/failures.c"
#include "support/authoring.c"
/** Execute every independent composition case. */
void run(void) {
    it_preserves_exact_failed_stage_diagnostics_and_invalid_project_ownership();
    it_keeps_native_authoring_contracts_separate_from_strict_frontend_composition();
    it_unwinds_every_complete_frontend_source_semantic_and_plan_allocation();
    it_matches_complete_frontend_outputs_to_independent_explicit_stages();
}
/** Return success only after every selected case passes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
