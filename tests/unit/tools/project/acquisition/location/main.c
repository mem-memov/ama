/** @file
 * @brief Compare physical directory normal forms with a finite independent word-state model.
 * @see [T234 contract](../../../../../../docs/implementation/acquisition-plan.md)
 */
#include "../common.c"
#include "../../../../../../tools/project/acquisition/location/internal.h"
/** Exhaust all 780 paths of one to four components over a,b,dot,parent,empty.
 * The oracle represents ordinary names in a bit sequence, not production byte
 * buffer positions. Root escape remains invalid even after later components. */
static void it_matches_every_bounded_relative_directory_word_model(void) {
    static const char *const words[] = {"a", "b", ".", "..", ""};
    size_t combinations = 5, checked = 0;
    for (size_t width = 1; width <= 4; width++, combinations *= 5) {
        for (size_t value = 0; value < combinations; value++) {
            begin(); checked++;
            char text[32] = {0}, expected[16] = {0};
            size_t used = 0, depth = 0; unsigned names = 0;
            size_t code = value; bool invalid = false;
            for (size_t index = 0; index < width; index++) {
                size_t symbol = code % 5; code /= 5;
                if (index) { text[used++] = '/'; }
                size_t bytes = strlen(words[symbol]); memcpy(text + used, words[symbol], bytes); used += bytes;
                if (!index && symbol == 4) { invalid = true; }
                if (symbol < 2) { names |= (unsigned)symbol << depth; depth++; }
                else if (symbol == 3) {
                    if (!depth) { invalid = true; }
                    else { depth--; names &= (1u << depth) - 1; }
                }
            }
            size_t length = 0;
            for (size_t index = 0; index < depth; index++) {
                if (index) { expected[length++] = '/'; }
                expected[length++] = names & (1u << index) ? 'b' : 'a';
            }
            SourceSpan original = configuration_test_text(text);
            const char *actual = "unchanged"; size_t actual_length = 999;
            AcquisitionMapStatus status = acquisition_directory(&results, original, &actual, &actual_length, &issue);
            if (invalid) { assert(status == ACQUISITION_MAP_INVALID && actual_length == 999 && !strcmp(actual, "unchanged")); }
            else { assert(status == ACQUISITION_MAP_OK && actual_length == length && !strcmp(actual, expected)); }
            assert(!memcmp(source_bytes(original.source), text, used));
            finish();
        }
    }
    assert(checked == 780);
}
/** The route's version boundary excludes src/tests/module suffixes from origin identity. */
static void it_maps_shared_versions_and_physical_dot_reductions_without_rerouting(void) {
    static const char *const directories[] = {".", "./", "base/./discard/../tree//", "目录/../данные"};
    static const char *const expected[] = {"p/v2.4.14-speed", "p/v2.4.14-speed", "base/tree/p/v2.4.14-speed", "данные/p/v2.4.14-speed"};
    static const char *const queries[] = {"/a/p/v2.4.14-speed/src/m", "/a/p/v2.4.14-speed/tests/m/test", "/a/p/v2.4.14-speed"};
    for (size_t directory = 0; directory < sizeof(directories) / sizeof(directories[0]); directory++) {
        begin();
        char text[512]; int written = snprintf(text, sizeof(text), "[repository /a]\nurl = origin\ndirectory = %s\n", directories[directory]);
        assert(written > 0 && (size_t)written < sizeof(text));
        ConfigurationDocument scope; ConfigurationStatus parsed = document(&scope, text, CONFIGURATION_PACKAGE); assert(parsed == CONFIGURATION_OK);
        for (size_t index = 0; index < sizeof(queries) / sizeof(queries[0]); index++) {
            AcquisitionRoute route;
            AcquisitionRouteStatus selected = acquisition_route_select(&scope, identity(queries[index]), &route, &issue); assert(selected == ACQUISITION_ROUTE_OK);
            const AcquisitionLocation *mapped = NULL;
            AcquisitionMapStatus status = acquisition_location(&results, &route, &mapped, &issue);
            assert(status == ACQUISITION_MAP_OK && mapped && mapped->route.scope == &scope && mapped->route.mapping == route.mapping && equal(mapped->route.mapping->url, "origin"));
            assert(!strcmp(mapped->version, "/a/p/v2.4.14-speed") && mapped->version_length == strlen(mapped->version));
            assert(!strcmp(mapped->directory, expected[directory]) && mapped->directory_length == strlen(expected[directory]));
        }
        configuration_document_destroy(&scope); finish();
    }
}
/** Every measured mapping allocation can refuse without publishing or poisoning inputs. */
static void it_retains_failed_mapping_scratch_and_recovers_with_the_same_borrowed_route(void) {
    size_t failures = 0;
    for (size_t failure = 0; failure == 0 || failure <= failures; failure++) {
        begin();
        ConfigurationDocument scope;
        ConfigurationStatus parsed = document(&scope, "[repository /a/p]\nurl = origin\ndirectory = root/../tree\n", CONFIGURATION_PACKAGE); assert(parsed == CONFIGURATION_OK);
        AcquisitionRoute route;
        AcquisitionRouteStatus selected = acquisition_route_select(&scope, identity("/a/p/v1.0.0/src/m"), &route, &issue); assert(selected == ACQUISITION_ROUTE_OK);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        AcquisitionLocation sentinel = {0}; const AcquisitionLocation *mapped = &sentinel;
        AcquisitionMapStatus status = acquisition_location(&results, &route, &mapped, &issue);
        if (!failure) { assert(status == ACQUISITION_MAP_OK); failures = allocation_count - before; assert(failures == 2); }
        else {
            assert(status == ACQUISITION_MAP_OUT_OF_MEMORY && mapped == &sentinel && issue.span.source == scope.source);
            failure_at = 0;
            status = acquisition_location(&results, &route, &mapped, &issue);
            assert(status == ACQUISITION_MAP_OK && !strcmp(mapped->directory, "tree/v1.0.0"));
        }
        configuration_document_destroy(&scope); finish();
    }
}
/** Bounded malformed directory bytes cannot become an escaping or truncated location. */
static void it_refuses_zero_absolute_and_missing_directory_metadata(void) {
    begin();
    static const unsigned char bytes[] = {'a', 0, 'b'};
    SourceSpan cases[] = {configuration_test_span(bytes, sizeof(bytes)), configuration_test_text("/root"), (SourceSpan){0}};
    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); index++) {
        const char *output = "untouched"; size_t length = 123;
        AcquisitionMapStatus status = acquisition_directory(&results, cases[index], &output, &length, &issue);
        assert(status == ACQUISITION_MAP_INVALID && length == 123 && !strcmp(output, "untouched"));
    }
    const AcquisitionLocation *output = NULL;
    AcquisitionMapStatus status = acquisition_location(&results, NULL, &output, &issue);
    assert(status == ACQUISITION_MAP_INVALID && !output);
    finish();
}
/** Run model, route composition and exact resource/error partitions explicitly. */
void run(void) {
    it_matches_every_bounded_relative_directory_word_model();
    it_maps_shared_versions_and_physical_dot_reductions_without_rerouting();
    it_retains_failed_mapping_scratch_and_recovers_with_the_same_borrowed_route();
    it_refuses_zero_absolute_and_missing_directory_metadata();
}
/** Exit after every result/metadata/source owner has retired cleanly. */
int main(void) { run(); return 0; }
