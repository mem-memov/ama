/** @file
 * @brief Verify exact editor lexer categories, raw comments/literals and invalid draft diagnostics.
 */
#include "../common.c"
static size_t comments, names, literals, visits;
/** Inspect original complete-name/literal/comment bytes without regenerating source text. */
static bool highlight(void *context, const EditorHighlight *item) {
    (void)context;
    visits++;
    assert(source_span_valid(item->spelling));
    if (item->comment) { comments++; }
    else if (item->kind == LEX_NAME && text_is(item->spelling, "multi word")) { names++; }
    else if (item->kind == LEX_LITERAL) {
        assert(text_is(item->payload, "#  literal  "));
        literals++;
    }
    return true;
}
/** A visitor can request bounded output without invalidating retained data or reporting a lexer error. */
static bool stop(void *context, const EditorHighlight *item) {
    (void)item;
    size_t *count = context;
    (*count)++;
    return false;
}
/** Drafts remain intentionally opaque even when their declaration text is incomplete. */
static void it_highlights_drafts_with_production_multiword_literal_and_comment_boundaries(void) {
    project_fixture("multi word # trailing\r\n # indented\r\n|#  literal  \r\n");
    setup_editor();
    assert(analyze(&analysis) && analysis.state == EDITOR_DRAFT && !analysis.indexed);
    comments = names = literals = visits = 0;
    Diagnostic diagnostic = {0};
    assert(editor_highlights(&analysis, "/test/fixture/v1.0.0/src/main", highlight, NULL, &diagnostic));
    assert(comments == 2 && names == 1 && literals == 1 && visits == 4);
    size_t count = 0;
    assert(editor_highlights(&analysis, "/test/fixture/v1.0.0/src/main", stop, &count, NULL) && count == 1);
    assert(point(analysis.selected->text,"multi word") == 0);
    destroy_editor();
}
/** Active syntax failure exposes the original compiler diagnostic, while valid lexical prefix tokens remain available. */
static void it_keeps_invalid_code_distinct_from_checked_semantic_metadata(void) {
    const char *invalid[] = {"@!\nvalue\n", "unfinished  name\n", "unfinished\tname\n"};
    for (size_t index = 0; index < sizeof(invalid)/sizeof(*invalid); index++) {
        project_fixture(invalid[index]);
        setup_editor();
        assert(analyze(&analysis) && analysis.state == EDITOR_INVALID && !analysis.indexed);
        assert(diagnostic_valid(&analysis.diagnostic));
        EditorDefinition output = {.type=(const SemanticType *)&analysis};
        assert(!editor_definition(&analysis, "/test/fixture/v1.0.0/src/main",0,&output) && output.type == (const SemanticType *)&analysis);
        visits = comments = names = literals = 0;
        Diagnostic error = {0};
        bool lexical = editor_highlights(&analysis, "/test/fixture/v1.0.0/src/main",highlight,NULL,&error);
        assert(lexical == (index == 0));
        if (!lexical) { assert(diagnostic_valid(&error)); }
        destroy_editor();
    }
}
/** Execute lexical/draft contracts without inventing recovered AST or unresolved symbols. */
void run(void) {
    it_highlights_drafts_with_production_multiword_literal_and_comment_boundaries();
    it_keeps_invalid_code_distinct_from_checked_semantic_metadata();
}
/** Return only after all counted partial owners and source views have retired. */
int main(void) { run(); return EXIT_SUCCESS; }
