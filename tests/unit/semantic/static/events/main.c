/** @file
 * @brief Verify structural static contexts, source order, owner traversal and failure cleanup.
 */
#include "../common.c"
#include "semantic/static/events/internal.h"
#include <stdint.h>

#include "support/contexts.c"
#include "support/ordering.c"
#include "support/failures.c"
#include "support/depth.c"

/** Run each actual collector/sorter component case explicitly. */
void run(void) {
    it_collects_exact_static_heads_and_receivers_in_all_source_contexts();
    it_visits_every_runtime_owning_subtree_once_without_following_alias_links();
    it_orders_every_five_record_permutation_and_prefix_stably_without_allocation();
    it_unwinds_every_occurrence_collection_allocation_failure_and_checked_count();
    it_collects_deep_expression_owners_without_recursive_c_activation();
}

/** Return success only after all ownership and assertion checks. */
int main(void) { run(); return EXIT_SUCCESS; }
