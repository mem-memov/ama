/**
 * @file
 * @brief Verify bounded INI recognition and exact alias/selector/repository shapes.
 * The [configuration contract](../../../../../docs/implementation/project-configuration.md)
 * separates recognition from profile tables and later source resolution.
 */
#include "../common.c"
#include "source/configuration/records/internal.h"
#include "support/read.c"
#include "support/shapes.c"

/** Run every independent record and shape boundary case explicitly. */
void run(void) {
    it_recognizes_exact_records_and_preserves_raw_value_views();
    it_refuses_malformed_headers_and_assignment_joins_without_publication();
    it_checks_alias_keys_and_profile_specific_raw_targets();
    it_checks_selector_components_without_resolving_or_cancelling_masks();
    it_separates_logical_repository_prefixes_from_physical_directory_containment();
}

/** Return only after immutable sources and all counted storage have retired. */
int main(void) { run(); return EXIT_SUCCESS; }
