/** @file
 * @brief Run finite geometry, byte-preservation, occurrence-projection and failure ownership cases.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "common.c"
#include "add-cases.c"
#include "apply-cases.c"
#include "project-cases.c"
/** Explicit runner keeps each invariant's acceptance visible in the leaf. */
void run(void) {
    it_exhausts_all_finite_range_and_insertion_overlap_partitions();
    it_owns_replacements_and_retires_each_add_allocation_failure();
    it_refuses_invalid_edit_interfaces_and_unrepresentable_lengths();
    it_applies_every_edit_permutation_to_the_same_exact_prepared_bytes();
    it_handles_empty_input_complete_deletion_and_unchanged_opaque_bytes();
    it_retires_every_prepared_output_allocation_failure_and_invalid_apply_interface();
    it_projects_all_original_occurrences_using_exact_replacement_and_insertion_boundaries();
    it_preserves_outside_boundary_insertions_and_requires_exact_prepared_owners();
}
/** Every case retires owners before successful generated-directory cleanup. */
int main(void) { run(); return 0; }
