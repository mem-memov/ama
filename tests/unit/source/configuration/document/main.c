/**
 * @file
 * @brief Check complete strict INI tables, original diagnostics and all arena refusals.
 * The [configuration contract](../../../../../docs/implementation/project-configuration.md)
 * keeps these source-only tests independent of filesystem/alias resolution.
 */
#include <stdio.h>
#include "../common.c"
#include "source/configuration/document/document.h"

static ConfigurationDocument document;

/** Initialize a fresh document over an actual immutable text fixture. */
static ConfigurationStatus parse(const char *text, ConfigurationProfile profile) {
    SourceSpan input = configuration_test_text(text);
    configuration_document_init(&document, sources.allocator);
    return configuration_document_parse(&document, input.source, profile);
}

/** Independently compare complete observed bytes, without the production key comparator. */
static bool text_is(SourceSpan span, const char *expected) {
    return source_span_valid(span) && span.end - span.start == strlen(expected)
        && memcmp(source_bytes(span.source) + span.start, expected, strlen(expected)) == 0;
}

/** Retire parsed records before borrowed source bytes and assert counted cleanup. */
static void finish(void) {
    configuration_document_destroy(&document);
    destroy_test();
}

#include "support/valid.c"
#include "support/invalid.c"
#include "support/ownership.c"

/** Run complete schema, ordering, byte and failure partitions explicitly. */
void run(void) {
    it_accepts_empty_profiles_and_empty_optional_sections();
    it_preserves_exact_alias_selector_and_raw_repository_values();
    it_accepts_all_section_and_repository_field_permutations();
    it_accepts_long_records_without_fixed_buffers_or_recursive_path_scanning();
    it_rejects_malformed_profile_records_and_duplicate_tables();
    it_reports_original_duplicate_unknown_and_incomplete_record_sites();
    it_cleans_every_document_allocation_refusal_without_touching_source();
    it_retains_bounded_diagnostics_for_every_byte_prefix();
    it_checks_complete_encoding_before_constructing_any_records();
}

/** Return only when all source/arena owners have retired without leaks. */
int main(void) { run(); return EXIT_SUCCESS; }
