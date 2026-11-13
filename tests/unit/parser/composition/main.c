/** @file
 * @brief Exercise composed source-to-AST parsing using documented grammar families.
 */
#include <stdio.h>
#include "../ast-common.c"
#include "support/shape.c"
#include "support/examples.c"
#include "support/failures.c"
#include "support/documented.c"
/** Run explicit whole-grammar and source ownership scenarios. */
void run(void) {
    it_parses_every_canonical_ama_and_abi_text_example_in_its_declared_context();
    it_parses_documented_forms_with_exact_original_source_views();
    it_checks_every_byte_prefix_of_a_composed_unicode_program();
    it_unwinds_every_composed_source_to_ast_allocation_failure();
}
/** Return success only after every independent assertion case completes. */
int main(void) { run(); return EXIT_SUCCESS; }
