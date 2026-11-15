/** @file
 * @brief Determine the full active workspace using the production activation classifier, leaving drafts opaque.
 * [SYN-04](../../../spec/language/source-and-names.md#syn-04) excludes drafts from execution;
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) retains otherwise unused active importers.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
/** Total exact logical-path order gives deterministic required-module registration independent of filesystem enumeration. */
static int compare(const void *left, const void *right) {
    return strcmp(*(const char *const *)left,*(const char *const *)right);
}
/** A complete catalog bounds the temporary path array; layout nodes retire after each classification. */
bool editor_workspace_active(EditorAnalysis *analysis, const char *const **output, size_t *output_count) {
    size_t capacity = 0;
    for (const ModuleSource *source = analysis->frontend.project.catalog.first; source != NULL; source = source->next) {
        if (capacity == SIZE_MAX/sizeof(char *)) { return false; }
        capacity++;
    }
    const char **paths = source_arena_allocate(&analysis->arena,capacity*sizeof(*paths));
    if (paths == NULL) { return false; }
    size_t count = 0;
    for (const ModuleSource *source = analysis->frontend.project.catalog.first; source != NULL; source = source->next) {
        SourceLayout layout;
        layout_init(&layout,analysis->frontend.allocator);
        bool active = false;
        ParseError error = {0};
        bool valid = layout_read(&layout,source->text) == LAYOUT_OK;
        if (!valid) { analysis->diagnostic = layout.diagnostic; }
        else {
            valid = parser_module_classify(&layout,&active,&error);
            if (!valid) { analysis->diagnostic = error.diagnostic; }
        }
        layout_destroy(&layout);
        if (!valid) { return false; }
        if (active) { paths[count++] = source->logical_path; }
    }
    qsort(paths,count,sizeof(*paths),compare);
    *output = paths;
    *output_count = count;
    return true;
}
