/** @file
 * @brief Run actual private request ownership, protection, foreign-data preservation and failure cases explicitly.
 * @see [T287 entry review](../../../../../../docs/implementation/source-operations.md#t287-entry-review-after-accepted-t286)
 */
#define _XOPEN_SOURCE 700
#include "common.c"
#include "cases.c"
#include "refusals.c"
#include "failures.c"
/** Keep ownership/protection/resource partitions independently named and runnable. */
void run(void) {
    it_owns_actual_original_and_final_requests_outside_sources_and_cleans_them_safely();
    it_skips_protected_and_existing_private_names_before_any_creation();
    it_refuses_unavailable_or_protected_workspace_inputs_without_source_mutation();
    it_preserves_foreign_workspace_members_bytes_and_root_replacements();
    it_retires_workspace_allocation_and_cancelled_prefixes_with_primary_issues_intact();
}
/** Preserve generated evidence on assertion failure. */
int main(void) { run(); return 0; }
