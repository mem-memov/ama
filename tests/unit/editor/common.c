/** @file
 * @brief Own counted frontend/editor fixtures and exact source-point observations.
 * No native compilation is needed to inspect these empty nominal module contracts.
 */
#ifndef AMA_EDITOR_TEST_COMMON
#define AMA_EDITOR_TEST_COMMON
#include "../compiler/common.c"
#include "editor/analysis/analysis.h"
static EditorAnalysis analysis;
/** Analyze one fresh explicit project while retaining all earlier immutable snapshots independently. */
static inline bool analyze(EditorAnalysis *owner) {
    ModuleRoot root = {"generated/project",NULL,0,MODULE_ROOT_PROJECT};
    bool valid = editor_analysis_build(owner, &root, 1, "/test/fixture/v1.0.0/src/main");
    if ((owner->state == EDITOR_INVALID || !valid) && owner->diagnostic.code != DIAGNOSTIC_OUT_OF_MEMORY) {
        fprintf(stderr, "Editor analysis: %s\n", owner->diagnostic.message == NULL ? "unavailable" : owner->diagnostic.message);
    }
    return valid;
}
/** Borrow counted allocation authority; the unused compiler fixture remains empty and independently destructible. */
static void setup_editor(void) {
    setup_frontend();
    editor_analysis_init(&analysis, sources.allocator);
}
/** Editor views expire before counted source/allocator ownership. */
static void destroy_editor(void) {
    editor_analysis_destroy(&analysis);
    destroy_frontend();
}
/** Install nominal scalar/worker/shared contracts used by actual source binding checks. */
static void project_fixture(const char *main) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/cell", "@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/worker", "@<\nargument\n/test/fixture/v1.0.0/src/cell\n@>\nresult&\n/test/fixture/v1.0.0/src/cell\nitems[]\n/test/fixture/v1.0.0/src/cell\n@!\nprivate\n/test/fixture/v1.0.0/src/cell\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/palette", "@=\nshared\n/test/fixture/v1.0.0/src/cell\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", main);
}
/** Find one exact occurrence in retained original bytes; fixtures deliberately choose unambiguous needles. */
static size_t point(const SourceText *source, const char *needle) {
    const char *found = strstr((const char *)source_bytes(source), needle);
    assert(found != NULL);
    return (size_t)(found - (const char *)source_bytes(source));
}
/** Source spans keep both exact spelling and their owning original file. */
static bool text_is(SourceSpan span, const char *text) { return layout_text_is(span, text); }
#endif
