/** @file
 * @brief Prepare genuine standard and versioned command originals for seed composition tests.
 * @see [T290](../../../../../../../docs/implementation/source-operations.md#t290-command-relation-construction-after-t292)
 */
#include "../../../../../../../tools/project/command/source/seeds/internal.h"
/** Fixed fixture paths are independent from the production's unbounded path handling. */
static const char *seed_version = "generated/project/packages/vendor/family/pkg/v1.0.0";
static ProjectSourceRequest seed_request;
static ProjectSourcePreparation seed_preparation;
static char seed_source[512], seed_destination[512];
static const char *seed_arguments[10];
/** Bound fixture spelling and check truncation; production paths use arenas. */
static void seed_spelling(char *output, size_t capacity, const char *left, const char *right) {
    int length = snprintf(output, capacity, "%s%s", left, right);
    assert(length >= 0 && (size_t)length < capacity);
}
/** Create an actual nested package and reciprocal reference definitions, never
 * fabricated compiler/closure records. Draft profiles exercise indexed non-code. */
static void seed_package(bool active) {
    const char *directories[] = {"generated/project", "generated/project/packages", "generated/project/packages/vendor",
        "generated/project/packages/vendor/family", "generated/project/packages/vendor/family/pkg",
        seed_version, "generated/project/packages/vendor/family/pkg/v1.0.0/src",
        "generated/project/packages/vendor/family/pkg/v1.0.0/tests",
        "generated/project/packages/vendor/family/pkg/v1.0.0/tests/a", "generated/project/packages/vendor/family/pkg/v1.0.0/tests/b"};
    for (size_t i = 0; i < sizeof(directories) / sizeof(*directories); i++) { assert(mkdir(directories[i], 0700) == 0); }
    char path[512]; seed_spelling(path, sizeof(path), seed_version, "/package.ini"); file(path, "");
    seed_spelling(path, sizeof(path), seed_version, "/src/a"); file(path, active ? "@!\nnext&\n./b\n" : "");
    seed_spelling(path, sizeof(path), seed_version, "/src/b"); file(path, active ? "@!\nnext&\n./a\n" : "");
    seed_spelling(path, sizeof(path), seed_version, "/tests/a/check"); file(path, active ? "@!\nvalue&\n/vendor/family/pkg/v1.0.0/src/a\n" : "@!\n");
    seed_spelling(path, sizeof(path), seed_version, "/tests/b/check"); file(path, active ? "@!\nvalue&\n/vendor/family/pkg/v1.0.0/src/b\n" : "@!\n");
}
/** Profiles: standard main, same-version draft, forbidden cross-version draft,
 * explicit extension, dependency cloning with a real cycle, raw opaque data,
 * and two explicit draft mains sharing one version authority. */
static void seed_fixture_begin(unsigned profile) {
    setup_test(); assert(mkdir("generated/scratch", 0700) == 0);
    bool ordinary = profile != 0 && profile != 5;
    if (ordinary) {
        seed_package(profile == 4);
        seed_spelling(seed_source, sizeof(seed_source), seed_version, profile == 6 ? "" : "/src/a");
        seed_spelling(seed_destination, sizeof(seed_destination), profile < 2 ? seed_version
            : "generated/project/packages/vendor/family/pkg/v2.0.0", profile == 6 ? "" : "/src/copied");
    } else {
        assert(mkdir("generated/source", 0700) == 0); file("generated/source/first", "@!\n");
        if (profile == 0) { file("generated/source/existing", "@!\n"); assert(mkdir("generated/source/bad", 0700) == 0); }
        if (profile == 5) { assert(mkdir("generated/source/data", 0700) == 0); file("generated/source/data/blob.bin", "opaque"); }
        seed_spelling(seed_source, sizeof(seed_source), "generated/source/", profile == 5 ? "data" : "first");
        seed_spelling(seed_destination, sizeof(seed_destination), "generated/source/", "copied");
    }
    const char *arguments[] = {"ama-project", "copy", seed_source, seed_destination, "--preview",
        ordinary ? "--root" : "--standard-root", ordinary ? "generated/project" : "generated/source",
        "--compiler", "../../../../../../ama", profile == 4 ? "--copy-dependencies" : "--extend-package"};
    memcpy(seed_arguments, arguments, sizeof(arguments));
    project_source_request_init(&seed_request, sources.allocator);
    assert(project_source_request_parse(&seed_request, profile == 3 || profile == 4 || profile == 6 ? 10 : 9, seed_arguments) == PROJECT_SOURCE_REQUEST_OK);
    project_source_preparation_init(&seed_preparation, sources.allocator);
    ProjectSourcePreparationInputs inputs = {.request = &seed_request, .temporary_parent = "generated/scratch"};
    ProjectSourcePreparationStatus captured = project_source_preparation_capture(&seed_preparation, &inputs);
    if (captured != PROJECT_SOURCE_PREPARATION_OK) {
        fprintf(stderr, "seed profile %u capture status %d\n", profile, captured);
        if (seed_preparation.original.original.diagnostics) {
            const SourceText *diagnostic = seed_preparation.original.original.diagnostics;
            assert(fwrite(source_bytes(diagnostic), 1, source_length(diagnostic), stderr) == source_length(diagnostic));
        }
    }
    assert(captured == PROJECT_SOURCE_PREPARATION_OK);
}
/** Only known test-created originals are removed; the product must create no
 * destination version/tree and must retain no unaccounted allocation/descriptor. */
static void seed_fixture_end(unsigned profile) {
    assert(access(seed_destination, F_OK) != 0 && errno == ENOENT);
    assert(source_workspace_cleanup(&seed_preparation.workspace) == SOURCE_WORKSPACE_OK);
    assert(project_source_preparation_destroy(&seed_preparation) == 0); project_source_request_destroy(&seed_request);
    if (profile != 0 && profile != 5) {
        const char *files[] = {"/package.ini", "/src/a", "/src/b", "/tests/a/check", "/tests/b/check"};
        char path[512];
        for (size_t i = 0; i < sizeof(files) / sizeof(*files); i++) { seed_spelling(path, sizeof(path), seed_version, files[i]); assert(unlink(path) == 0); }
        const char *directories[] = {"/tests/a", "/tests/b", "/src", "/tests", ""};
        for (size_t i = 0; i < sizeof(directories) / sizeof(*directories); i++) { seed_spelling(path, sizeof(path), seed_version, directories[i]); assert(rmdir(path) == 0); }
        assert(rmdir("generated/project/packages/vendor/family/pkg") == 0 && rmdir("generated/project/packages/vendor/family") == 0
            && rmdir("generated/project/packages/vendor") == 0 && rmdir("generated/project/packages") == 0 && rmdir("generated/project") == 0);
    } else {
        if (profile == 5) { assert(unlink("generated/source/data/blob.bin") == 0 && rmdir("generated/source/data") == 0); }
        if (profile == 0) { assert(unlink("generated/source/existing") == 0 && rmdir("generated/source/bad") == 0); }
        assert(unlink("generated/source/first") == 0 && rmdir("generated/source") == 0);
    }
    assert(rmdir("generated/scratch") == 0); destroy_test();
}
