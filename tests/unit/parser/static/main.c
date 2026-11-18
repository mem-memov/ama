/**
 * @file
 * @brief Verify static source spellings and structural trees without symbol lookup.
 */
#include "../ast-common.c"
#include "parser/internal.h"
#include "ast/static.h"
#include <stdio.h>

#include "support/names.c"
#include "support/expressions.c"
#include "support/interfaces.c"

/** Explicitly run each component case; failed assertions preserve leaf outputs. */
void run(void) {
    it_recognizes_atomic_static_names_without_module_lookup();
    it_preserves_nested_expression_owners_and_initializer_boundaries();
    it_retains_positional_input_and_output_skips_as_distinct_bindings();
    it_rejects_static_operand_order_and_indentation_errors_without_publication();
    it_parses_deep_static_trees_and_unwinds_every_expression_allocation_failure();
    it_composes_static_interfaces_with_runtime_declarations_in_both_implementations();
    it_composes_projections_constant_reads_and_initialized_type_expressions();
    it_rejects_incomplete_static_interfaces_and_execution_suffixes();
    it_unwinds_every_composed_static_module_allocation_failure();
    it_parses_deep_signatures_without_c_recursion();
    it_preserves_output_only_interface_heads_and_explicit_type_controls();
}

/** Return success only after all structural assertions and ownership cleanup. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
