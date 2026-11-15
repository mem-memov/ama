/** @file
 * @brief Execute complete discovery, exact endpoint/root projection and failure cleanup boundaries.
 */
#include "../common.c"
#include "tools/relocation/internal.h"
#include "support/scope.c"
#include "support/paths.c"
#include "support/roots.c"
#include "support/failures.c"

/** Keep every required preflight scenario explicit in the independently runnable leaf. */
void run(void) {
    it_discovers_all_sources_and_preserves_opaque_accompanying_bytes();
    it_uses_exact_destination_and_rejects_unsafe_endpoints();
    it_resolves_long_working_directories_and_checks_required_parent_permissions();
    it_projects_carried_roots_and_explicit_destination_root_kinds();
    it_moves_between_existing_root_kinds_and_includes_destination_sources();
    it_previews_constants_only_and_empty_source_domains_without_an_entry();
    it_rejects_collisions_aliases_and_orphan_plus_directories();
    it_rejects_invalid_unused_active_sources_but_accepts_draft_bodies();
    it_releases_every_failed_preparation_prefix_without_touching_files();
    it_preserves_io_diagnostics_without_allocating_during_failure_reporting();
}
/** Every owned failure prefix must retire before a successful process exit. */
int main(void) { run(); return EXIT_SUCCESS; }
