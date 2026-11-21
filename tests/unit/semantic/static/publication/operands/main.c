/** @file
 * @brief Run source-only complete operand facts, deep collection and resource-failure cases.
 */
#include "../common.c"
#include "semantic/static/publication/ordinary/inputs/internal.h"
#include "support/prepare.c"
#include "support/facts.c"
#include "support/failures.c"

/** Explicitly execute the algebraic inference, cache and ownership partitions. */
static void run(void) {
    it_infers_owner_types_from_nested_ordinary_operand_facts_without_actual_roots();
    it_keeps_branched_operand_sources_distinct_and_matches_reordered_named_maps();
    it_collects_empty_and_deep_input_trees_once_without_equations_or_body_work();
    it_unwinds_every_nested_input_collection_and_equation_allocation_failure();
}

/** Counted teardown must complete before the harness succeeds. */
int main(void) { run(); return 0; }
