/** @file
 * @brief Run actual-owner key, role registration and managed-refusal component contracts.
 * @see [T188 contract](../../../../../docs/implementation/specialized-execution.md)
 */
#include "support/fixture.c"
#include "support/keys.c"
#include "support/failures.c"

/** Run every independent identity and failure partition explicitly. */
static void run(void) {
    it_preserves_full_owner_role_keys_and_concrete_native_namespace_identity();
    it_refuses_foreign_incomplete_and_aliased_owner_keys_without_changing_output();
    it_refuses_native_namespace_category_for_captured_ordinary_owners();
    it_unwinds_every_owner_key_allocation_without_publishing_partial_keys();
    it_unwinds_every_scaffold_owner_role_registration_allocation_and_preserves_first_failure();
}

/** Succeed only after all assertions and counted owner teardown. */
int main(void) { run(); return 0; }
