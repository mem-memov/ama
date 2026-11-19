/** @file
 * @brief Check exact concrete dependency extraction independently of parsing and loading.
 */
#include "project/dependencies/discover/internal.h"

/** An explicit source slice stays with the independent source manager until
 * project teardown has released every borrowed request record.
 */
static SourceSpan static_path(const SourceText *source, size_t start, const char *text) {
    SourceSpan span = {source, start, start + strlen(text)};
    assert(source_span_valid(span));
    assert(memcmp(source_bytes(source) + start, text, strlen(text)) == 0);
    return span;
}

/** A placeholder root still owns concrete nested operands; receivers and export selectors
 * add none, while separately written equal paths retain separate exact sites.
 */
static void it_extracts_only_concrete_static_heads_and_preserves_occurrence_sites(void) {
    setup_project();
    const char *text = ".outer\n..inner\n...family\n.outer\n...alias\n";
    const SourceText *source;
    assert(source_manager_add(&sources, "static-dependencies", (const unsigned char *)text,
        strlen(text), &source) == SOURCE_OK);
    AstTypeExpression inner = {.head = static_path(source, 7, "..inner"), .kind = AST_TYPE_MODULE_PATH};
    AstStaticBinding inner_binding = {.kind = AST_STATIC_INPUT, .input = &inner};
    AstTypeExpression outer = {.head = static_path(source, 0, ".outer"),
        .kind = AST_TYPE_MODULE_PATH, .first = &inner_binding, .last = &inner_binding};
    AstTypeExpression repeated = {.head = static_path(source, 25, ".outer"), .kind = AST_TYPE_MODULE_PATH};
    AstStaticBinding output = {.kind = AST_STATIC_OUTPUT, .receiver = static_path(source, 32, "...alias")};
    AstStaticBinding second = {.kind = AST_STATIC_INPUT, .input = &repeated, .next = &output};
    AstStaticBinding first = {.kind = AST_STATIC_INPUT, .input = &outer, .next = &second};
    AstTypeExpression root = {.head = static_path(source, 15, "...family"),
        .kind = AST_TYPE_PLACEHOLDER, .first = &first, .last = &output};
    DependencyPaths paths = {0};
    size_t allocations = allocation_count;
    assert(project_collect_expression(&project, &paths, NULL));
    assert(paths.count == 0 && allocation_count == allocations);
    assert(project_collect_expression(&project, &paths, &root));
    assert(paths.count == 3);
    project_sort_paths(&paths);
    SourceSpan expected[] = {outer.head, inner.head, repeated.head};
    const DependencyPath *request = paths.first;
    for (size_t index = 0; index < 3; index++) {
        assert(request != NULL && request->implicit == NULL);
        assert(request->path.source == source && request->path.start == expected[index].start
            && request->path.end == expected[index].end);
        request = request->next;
    }
    assert(request == NULL);
    destroy_project();
}
