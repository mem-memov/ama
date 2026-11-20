/** @file
 * @brief Exercise independent final parsing and graph validation without filesystem publication.
 */
#include "../common.c"
#include "tools/relocation/staging/staging.h"
#include "tools/relocation/staging/internal.h"

/** Find one prepared source through its explicit original identity, never filesystem guesses. */
static const RelocationStagedSource *prepared(const RelocationStaging *staging, const char *identity) {
    const RelocationModule *original = module(identity);
    for (const RelocationStagedSource *source = staging->first; source != NULL; source = source->next) {
        if (source->module == original) { return source; }
    }
    return NULL;
}
/** Require exact complete bytes, including physical line endings and optional final newline. */
static void exact(const SourceText *text, const char *expected) {
    assert(text != NULL && source_length(text) == strlen(expected));
    assert(memcmp(source_bytes(text),expected,strlen(expected)) == 0);
}
#include "support/static.c"
#include "support/graph.c"
#include "support/files.c"
#include "support/settings.c"
#include "support/failures.c"
#include "support/text-model.c"
#include "support/domain.c"
/** Run staged source/graph checks with real production parser and semantics. */
void run(void) {
    it_relocates_static_producers_without_rewriting_atomic_names_or_returned_origins();
    it_relocates_nested_constructor_origins_without_collapsing_concrete_captures();
    it_rejects_opaque_physical_aliases_with_retained_locations();
    it_validates_newly_covered_sources_and_empty_domains();
    it_matches_the_complete_two_span_text_model();
    it_rejects_redirected_final_bindings_and_changed_original_bytes();
    it_releases_every_failed_staging_prefix_and_preserves_originals();
    it_prepares_explicit_settings_and_generated_output_invalidations();
    it_captures_native_assets_links_and_validates_qualified_constants();
    it_validates_final_cycles_and_preserves_all_nonpath_bytes();
}
/** Every independently initialized fixture owner retires before process exit. */
int main(void) { run(); return EXIT_SUCCESS; }
