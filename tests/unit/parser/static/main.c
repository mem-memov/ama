/**
 * @file
 * @brief Verify static source spellings and structural trees without symbol lookup.
 */
#include "../ast-common.c"
#include "parser/internal.h"
#include "ast/static.h"
#include <stdio.h>

#include "support/names.c"
#include "support/graphs.c"
#include "support/ordinary.c"
#include "support/failures.c"

/** Explicitly run each component case; failed assertions preserve leaf outputs. */
void run(void) {
    it_recognizes_atomic_static_names_without_module_lookup();
    it_keeps_empty_static_graphs_separate_from_every_runtime_section();
    it_preserves_named_pairs_partial_values_and_multiple_producer_outputs();
    it_obeys_graph_comment_regions_and_does_not_reopen_closed_children();
    it_keeps_simple_ordinary_type_pairs_and_export_initializers();
    it_selects_constant_namespaces_without_ordinary_static_applications();
    it_rejects_old_static_forms_and_invalid_graph_modes();
    it_rejects_invalid_ordinary_selectors_and_nested_static_blocks();
    it_parses_deep_input_only_values_without_c_recursion();
    it_unwinds_every_current_graph_and_selector_parse_allocation_failure();
}

/** Return success only after all structural assertions and ownership cleanup. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
