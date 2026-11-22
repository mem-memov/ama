/** @file
 * @brief Check exact mirrored test paths, version authority and refusal atomicity.
 * @see [T235 contract](../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
#include "../closure/common.c"
#include "../../../../../../tools/project/acquisition/closure/tests/association.h"
/** Physical '+' placement is intentionally absent from this operation's inputs;
 * nested logical components and Unicode names preserve all bytes. */
static void it_maps_nested_logical_suffixes_in_the_selected_version(void) {
    static const char *const suffixes[] = {"sort", "sort/partition", "sort/partition/pivot", "дерево/ключ ключ"};
    static const char *const roots[] = {"selected/packages/a/p/v2.4.14-speed", "selected/packages/a/p/v2.4.14-speed/"};
    for (size_t root = 0; root < 2; root++) {
        for (size_t index = 0; index < 4; index++) {
            begin();
            ModuleVersion version = {.logical_path = "/a/p/v2.4.14-speed", .physical_path = roots[root]};
            char identity[256], expected_logical[256], expected_physical[256];
            int n = snprintf(identity, sizeof(identity), "%s/src/%s", version.logical_path, suffixes[index]); assert(n > 0 && (size_t)n < sizeof(identity));
            n = snprintf(expected_logical, sizeof(expected_logical), "%s/tests/%s", version.logical_path, suffixes[index]); assert(n > 0 && (size_t)n < sizeof(expected_logical));
            n = snprintf(expected_physical, sizeof(expected_physical), "selected/packages/a/p/v2.4.14-speed/tests/%s", suffixes[index]); assert(n > 0 && (size_t)n < sizeof(expected_physical));
            ModuleSource source = {.logical_path = identity, .version = &version};
            AcquisitionTestDirectory result = {0};
            SourceStatus status = acquisition_test_directory(&results, &source, &result);
            assert(status == SOURCE_OK && !strcmp(result.logical, expected_logical) && !strcmp(result.physical, expected_physical));
            identity[1] = 'z'; assert(result.logical[1] == 'a');
            finish();
        }
    }
}
/** Invalid endpoints and mismatched version owners cannot invent test locations. */
static void it_refuses_non_source_paths_and_mismatched_version_owners(void) {
    begin();
    ModuleVersion version = {.logical_path = "/a/p/v1.0.0", .physical_path = "selected"};
    static const char *const paths[] = {"//integer", "/a/p/v1.0.0/tests/check", "/a/p/v1.0.0/src", "/a/p/v1.0.0/src/", "/a/p/v2.0.0/src/m", "/a/p/v1.0.0/src/m+/child"};
    for (size_t index = 0; index < sizeof(paths) / sizeof(paths[0]); index++) {
        ModuleSource source = {.logical_path = paths[index], .version = &version};
        AcquisitionTestDirectory output = {.logical = "old logical", .physical = "old physical"};
        SourceStatus status = acquisition_test_directory(&results, &source, &output);
        assert(status == SOURCE_INVALID_ARGUMENT && !strcmp(output.logical, "old logical") && !strcmp(output.physical, "old physical"));
    }
    finish();
}
/** Either owned path allocation can refuse without changing the public pair. */
static void it_preserves_both_outputs_on_each_allocation_failure(void) {
    size_t allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        begin(); ModuleVersion version = {.logical_path = "/a/p/v1.0.0", .physical_path = "selected"};
        ModuleSource source = {.logical_path = "/a/p/v1.0.0/src/m", .version = &version};
        AcquisitionTestDirectory output = {.logical = "old logical", .physical = "old physical"};
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceStatus status = acquisition_test_directory(&results, &source, &output);
        if (!failure) { assert(status == SOURCE_OK); allocations = allocation_count - before; assert(allocations); }
        else {
            assert(status == SOURCE_OUT_OF_MEMORY && !strcmp(output.logical, "old logical") && !strcmp(output.physical, "old physical"));
            failure_at = 0;
            status = acquisition_test_directory(&results, &source, &output); assert(status == SOURCE_OK);
        }
        finish();
    }
}
/** Logical associations need no physical version object and preserve namespace
 * components exactly. Invalid/failing calls leave the caller's sentinel intact. */
static void it_derives_owned_test_namespaces_without_physical_version_handles(void) {
    begin();
    const char *output = "unchanged";
    static const char *const invalid[] = {NULL, "", "//integer", "/a/p/v1.0.0/src",
        "/a/p/v1.0.0/tests/check", "/a/p/v1.0.0/src/../m", "/a/p/v1.0.0/src/m+/child"};
    size_t before = allocation_count;
    for (size_t i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++) {
        assert(acquisition_test_namespace(&results, invalid[i], &output) == SOURCE_INVALID_ARGUMENT);
        assert(!strcmp(output, "unchanged") && allocation_count == before);
    }
    char input[] = "/a/p/v2.4.14-speed/src/дерево/ключ ключ";
    assert(acquisition_test_namespace(NULL, input, &output) == SOURCE_INVALID_ARGUMENT);
    assert(acquisition_test_namespace(&results, input, NULL) == SOURCE_INVALID_ARGUMENT);
    failure_at = allocation_count + 1;
    assert(acquisition_test_namespace(&results, input, &output) == SOURCE_OUT_OF_MEMORY);
    assert(!strcmp(output, "unchanged"));
    failure_at = 0;
    assert(acquisition_test_namespace(&results, input, &output) == SOURCE_OK);
    input[1] = 'z';
    assert(!strcmp(output, "/a/p/v2.4.14-speed/tests/дерево/ключ ключ"));
    finish();
}
/** Run association, nonmembership and owned-result failure partitions. */
void run(void) {
    it_derives_owned_test_namespaces_without_physical_version_handles();
    it_maps_nested_logical_suffixes_in_the_selected_version();
    it_refuses_non_source_paths_and_mismatched_version_owners();
    it_preserves_both_outputs_on_each_allocation_failure();
}
/** Exit after all source and output owners retire. */
int main(void) { run(); return 0; }
