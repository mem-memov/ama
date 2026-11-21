/**
 * @file
 * @brief Derive exact test-directory ancestry while checking physical/logical module correspondence.
 * [SRC-05](../../../../spec/language/source-and-names.md#src-05) strips exactly
 * one structural plus from directory components, never the main filename.
 * [PKG-08](../../../../spec/project/packages.md#pkg-08) associates ordinary
 * mirrored directories; the first plus ancestor ends their usable prefix.
 */
#include <string.h>
#include "internal.h"

DependencySelectionStatus dependency_view_provenance(const ModulePathValue *identity,
    const unsigned char *physical, size_t length, DependencyViewInput *output, SourceIssue *issue) {
    ModulePathShape shape;
    if (identity == NULL || physical == NULL || length == 0
        || !module_path_describe((const unsigned char *)identity->text, identity->length, &shape)
        || shape.root != MODULE_ROOT_PROJECT || shape.extent != MODULE_PATH_MODULE) { goto invalid; }
    size_t component = shape.version_components + 1;
    size_t ordinary = 0;
    bool structural = false;
    for (size_t cursor = 0; cursor < length;) {
        size_t end = cursor;
        while (end < length && physical[end] != '/') { end++; }
        bool directory = end < length;
        size_t width = end - cursor;
        bool plus = directory && width != 0 && physical[end - 1] == '+';
        size_t name_width = width - (plus ? 1 : 0);
        if (component == identity->count || name_width != identity->parts[component].length
            || memcmp(physical + cursor, identity->parts[component].bytes, name_width) != 0) { goto invalid; }
        component++;
        structural = structural || plus;
        if (directory && !structural) { ordinary++; }
        if (!directory) { break; }
        if (end + 1 == length) { goto invalid; }
        cursor = end + 1;
    }
    if (component != identity->count) { goto invalid; }
    *output = (DependencyViewInput){identity, shape.area == MODULE_PATH_TESTS ? ordinary : 0};
    return DEPENDENCY_SELECTION_OK;
invalid:
    return selection_failure(issue, identity == NULL ? (SourceSpan){0} : identity->expression,
        DEPENDENCY_SELECTION_INVALID, "Physical module path does not match its complete logical identity",
        "verified area-relative components with structural plus only on directories");
}
