/** @file
 * @brief Check exact identity-derived edits over the complete original semantic domain.
 */
#include "../common.c"
#include "tools/relocation/references/references.h"
#include "tools/relocation/references/internal.h"
#include "support/graph.c"
#include "support/new-scope.c"
#include "support/constants.c"
#include "support/failures.c"

/** Run independent graph, namespace/member preservation and failure ownership checks. */
void run(void) {
    it_analyzes_newly_covered_selected_sources_and_empty_domains();
    it_analyzes_new_scope_and_rejects_initial_identity_conflicts();
    it_maps_cycles_and_external_importers_without_textual_substitution();
    it_preserves_constant_members_suffixes_comments_and_literal_bytes();
    it_rejects_unresolved_original_bindings_and_conflicting_spans();
    it_releases_every_failed_reference_analysis_prefix();
}
/** All semantic/source/edit owners retire before successful process exit. */
int main(void) { run(); return EXIT_SUCCESS; }
