/**
 * @file
 * @brief Compare complete version boundaries and mirrored source/test directory suffixes.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) preserves exact version
 * spelling; [PKG-08](../../../../spec/project/packages.md#pkg-08) associates proper
 * descendants of tests/source-suffix, not a test file equal to that directory name.
 */
#include <string.h>
#include "internal.h"

bool selection_prefix(const char *prefix, size_t prefix_length, const char *value, size_t length, bool proper) {
    if (prefix_length > length || memcmp(prefix, value, prefix_length) != 0) { return false; }
    return prefix_length == length ? !proper : value[prefix_length] == '/';
}
bool selection_same_version(DependencyViewEntry a, DependencyViewEntry b) {
    return a.shape.root == MODULE_ROOT_PROJECT && b.shape.root == MODULE_ROOT_PROJECT
        && a.shape.version_length == b.shape.version_length
        && memcmp(a.identity->text, b.identity->text, a.shape.version_length) == 0;
}
bool selection_associated(DependencyViewEntry source, DependencyViewEntry test) {
    if (source.shape.extent != MODULE_PATH_MODULE || test.shape.extent != MODULE_PATH_MODULE
        || source.shape.area != MODULE_PATH_SRC || test.shape.area != MODULE_PATH_TESTS
        || !selection_same_version(source, test)) { return false; }
    size_t source_depth = source.shape.components - source.shape.version_components - 1;
    if (source_depth > test.test_directory_prefix) { return false; }
    size_t source_start = source.shape.version_length + sizeof("/src/") - 1;
    size_t test_start = test.shape.version_length + sizeof("/tests/") - 1;
    return selection_prefix(source.identity->text + source_start, source.identity->length - source_start,
        test.identity->text + test_start, test.identity->length - test_start, true);
}
