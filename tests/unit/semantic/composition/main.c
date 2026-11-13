/** @file
 * @brief Verify complete semantic composition, exact source metadata, deterministic stage equivalence and failure ownership.
 */
#include "../typed-common.c"
#include "support/fixture.c"
#include "support/compare.c"
#include "support/projects.c"
#include "support/failures.c"
/** Run positive complete projects, precise failed prefixes and every semantic allocation boundary. */
void run(void) {
    it_composes_every_current_construct_with_exact_source_and_role_metadata();
    it_matches_composite_and_explicit_stages_without_pointer_or_hash_identity_guesses();
    it_preserves_the_first_failed_stage_and_original_diagnostic_context();
    it_unwinds_every_allocation_of_complete_semantic_composition();
}
/** Return success only after every named composed assertion completes. */
int main(void) { run(); return EXIT_SUCCESS; }
