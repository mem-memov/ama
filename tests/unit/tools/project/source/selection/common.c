/** @file
 * @brief Build leaf-local original source trees for raw selection and deletion composition checks.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#ifndef AMA_SOURCE_SELECTION_TEST_COMMON
#define AMA_SOURCE_SELECTION_TEST_COMMON
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/selection/tree.h"
#include "../../../../../../tools/project/source/selection/internal.h"
#include "../../../../../../tools/project/source/selection/domain.h"
static SourceRootCapture capture;
static SourceTreeSelection selection;
static const char *main_path = "generated/project/packages/vendor/pkg/v1.0.0/src/parent";
static const char *subtree_path = "generated/project/packages/vendor/pkg/v1.0.0/src/parent+";
/** Exact fixture creation never hides unrelated filesystem failures. */
static void directory(const char *path) { assert(mkdir(path, 0700) == 0 || errno == EEXIST); }
/** Binary opaque bytes and source fixtures share byte-exact physical ownership. */
static void file(const char *path, const char *bytes) {
    FILE *output = fopen(path, "wb"); assert(output);
    assert(fwrite(bytes, 1, strlen(bytes), output) == strlen(bytes) && fclose(output) == 0);
}
/** This layer needs an original catalog, not executable admission or existing tests. */
static void selection_fixtures(void) {
    const char *directories[] = {"generated/project", "generated/project/packages", "generated/project/packages/vendor",
        "generated/project/packages/vendor/pkg", "generated/project/packages/vendor/pkg/v1.0.0",
        "generated/project/packages/vendor/pkg/v1.0.0/src", "generated/project/packages/vendor/pkg/v1.0.0/tests",
        "generated/project/packages/vendor/pkg/v1.0.0/src/parent+",
        "generated/project/packages/vendor/pkg/v1.0.0/src/parent+/empty", "generated/other",
        "generated/project/dependencies", "generated/project/dependencies/vendor", "generated/project/dependencies/vendor/pkg",
        "generated/project/dependencies/vendor/pkg/v1.0.0", "generated/project/dependencies/vendor/pkg/v1.0.0/src"};
    for (size_t index = 0; index < sizeof(directories) / sizeof(*directories); index++) { directory(directories[index]); }
    file("generated/project/packages/vendor/pkg/v1.0.0/package.ini", "");
    file(main_path, "unfinished parent\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/parent.ini", "; opaque local scope\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/parent+.data", "unrelated basename\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/child", "@!\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/parent+/opaque.data", "opaque bytes\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/src/parentish", "@!\n");
    file("generated/project/dependencies/vendor/pkg/v1.0.0/src/ignored", "\xff opaque ignored bytes\n");
    file("generated/project/packages/vendor/pkg/v7.8.9", "opaque version-shaped file\n");
    file("generated/project/packages/v4.5.6", "opaque vendor-position file\n");
}
/** Freeze prepared selection inputs without manufacturing post-capture variants. */
static void selection_capture_scope(bool exclude_child, bool standard_child) {
    source_root_capture_init(&capture, sources.allocator);
    const char *excluded[] = {"packages/vendor/pkg/v1.0.0/src/parent+"};
    ModuleRoot roots[] = {{"generated/project", exclude_child ? excluded : NULL, exclude_child ? 1u : 0u, MODULE_ROOT_PROJECT},
        {subtree_path, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(source_root_capture_build(&capture, roots, standard_child ? 2 : 1) == SOURCE_CAPTURE_OK);
    source_tree_selection_init(&selection, sources.allocator);
}
/** Build the ordinary/excluded/standard fixture before any operation begins. */
static void prepare_scope(bool exclude_child, bool standard_child) {
    setup_test(); selection_fixtures(); selection_capture_scope(exclude_child, standard_child);
}
/** Usual fixture selects only the project namespace with no exclusions. */
static void prepare(void) { prepare_scope(false, false); }
/** Borrowed original handles outlive every selected raw byte and projected array. */
static void finish(void) {
    source_tree_selection_destroy(&selection); source_root_capture_destroy(&capture); destroy_test();
}

#endif
