/** @file
 * @brief Own parsed request scopes and separate metadata/result arenas for pure acquisition tests.
 * @see [T234 contract](../../../../../docs/implementation/acquisition-plan.md)
 */
#ifndef AMA_ACQUISITION_TEST_COMMON
#define AMA_ACQUISITION_TEST_COMMON
#include <stdio.h>
#include "../../../source/configuration/common.c"
#include "../../../../../tools/project/acquisition/routing/route.h"
static SourceArena metadata, results;
static SourceIssue issue;
/** Own immutable source and normalized candidate metadata for one model case. */
static void begin(void) {
    setup_test(); source_arena_init(&metadata, sources.allocator);
    source_arena_init(&results, sources.allocator);
}
/** Retire normalized metadata after all borrowed route observations. */
static void finish(void) {
    source_arena_destroy(&results); source_arena_destroy(&metadata); destroy_test();
}
/** Produce an actual common-path normal form, retaining original source evidence. */
static const ModulePathValue *identity(const char *text) {
    SourceSpan expression = configuration_test_text(text);
    const ModulePathValue *output = NULL;
    ModulePathStatus status = module_path_normalize(&metadata, expression, NULL, NULL, NULL, &output, &issue);
    if (status != MODULE_PATH_OK) { fprintf(stderr, "Normalization refused %s: %s\n", text, issue.message); }
    assert(status == MODULE_PATH_OK); return output;
}
/** Parse an actual immutable requesting document rather than manufacturing tables. */
static ConfigurationStatus document(ConfigurationDocument *owner, const char *text, ConfigurationProfile profile) {
    SourceSpan input = configuration_test_text(text);
    configuration_document_init(owner, sources.allocator);
    return configuration_document_parse(owner, input.source, profile);
}
/** Compare observed original mapping values without using route-selection helpers. */
static inline bool equal(SourceSpan span, const char *text) {
    return source_span_valid(span) && span.end - span.start == strlen(text) &&
        !memcmp(source_bytes(span.source) + span.start, text, strlen(text));
}
#endif
