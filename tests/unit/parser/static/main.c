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
#include "support/declarations.c"
#include "support/failures.c"

/** Explicitly run each component case; failed assertions preserve leaf outputs. */
void run(void) {
    it_recognizes_atomic_static_names_without_module_lookup();
    it_classifies_only_valid_empty_static_modules_as_markers();
    it_distinguishes_marker_sources_from_other_and_invalid_files();
    it_preserves_named_pairs_partial_values_and_multiple_producer_outputs();
    it_obeys_graph_comment_regions_and_does_not_reopen_closed_children();
    it_keeps_simple_ordinary_type_pairs_and_export_initializers();
    it_selects_constant_namespaces_without_ordinary_static_applications();
    it_rejects_old_static_forms_and_invalid_graph_modes();
    it_rejects_invalid_selectors_and_namespace_static_bindings();
    it_parses_deep_input_only_values_without_c_recursion();
    it_preserves_nested_declaration_inputs_selectors_and_initializer_spans();
    it_parses_declaration_arguments_before_lookup_and_in_each_ordinary_section();
    it_rejects_crossed_expression_phases_outputs_and_nested_runtime_initializers();
    it_unwinds_every_current_graph_and_selector_parse_allocation_failure();
}

/** Return success only after all structural assertions and ownership cleanup. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
