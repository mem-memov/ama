/** @file
 * @brief Verify immutable changed-file snapshots, unavailable-input boundaries and every failed allocation prefix.
 */
#include "../common.c"
/** Count borrowed lexical names while validating every callback's original source range. */
static bool tokens(void *context, const EditorHighlight *token) {
    size_t *count = context;
    assert(source_span_valid(token->spelling));
    (*count)++;
    return true;
}
/** A fresh analysis owns changed bytes without rewriting any earlier declaration or lexical view. */
static void it_keeps_old_definitions_valid_while_analyzing_changed_unicode_source(void) {
    project_fixture("@!\nзначение данных\n.cell\ncopy\n значение данных\n");
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed);
    const SourceText *original = analysis.selected->text;
    EditorDefinition before;
    size_t use = point(original," значение данных")+1;
    assert(editor_definition(&analysis,".main",use,&before) && text_is(before.definition,"значение данных"));
    file("generated/project/main","@!\nnew value\n.cell\ncopy\n new value\n");
    EditorAnalysis changed;
    editor_analysis_init(&changed,sources.allocator);
    assert(analyze(&changed) && changed.indexed && changed.selected->text != original);
    EditorDefinition after;
    assert(editor_definition(&changed,".main",point(changed.selected->text," new value")+1,&after));
    assert(text_is(after.definition,"new value") && after.symbol != before.symbol);
    assert(text_is(before.definition,"значение данных") && before.definition.source == original);
    size_t count = 0;
    assert(editor_highlights(&analysis,".main",tokens,&count,NULL) && count != 0);
    editor_analysis_destroy(&changed);
    assert(editor_definition(&analysis,".main",use+1,&after) && after.symbol == before.symbol);
    destroy_editor();
}
/** A checked snapshot's lexical query failure cannot destroy its semantic index or retain temporary layout nodes. */
static void it_releases_temporary_highlight_failure_and_preserves_the_checked_snapshot(void) {
    project_fixture("@!\nvalue\n.cell\ncopy\n value\n");
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed);
    size_t live = live_allocations, count = 0;
    failure_at = allocation_count+1;
    Diagnostic diagnostic = {0};
    assert(!editor_highlights(&analysis,".main",tokens,&count,&diagnostic));
    assert(diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY && live_allocations == live && analysis.indexed);
    failure_at = 0;
    assert(editor_highlights(&analysis,".main",tokens,&count,NULL) && count != 0 && live_allocations == live);
    assert(point(analysis.selected->text,"value") != 0);
    destroy_editor();
}
/** Enumerate the exact successful construction count, then fail each ordinal rather than sampling guessed prefixes. */
static void it_releases_every_failed_snapshot_and_occurrence_index_construction_prefix(void) {
    project_fixture("@!\nvalue\n.cell\ncopy\n value\n");
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed);
    size_t total = allocation_count;
    destroy_editor();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_editor();
        failure_at = failure;
        bool valid = analyze(&analysis);
        if (valid || analysis.indexed || analysis.diagnostic.code != DIAGNOSTIC_OUT_OF_MEMORY) {
            fprintf(stderr,"Editor failure ordinal %zu/%zu: valid=%u indexed=%u code=%u\n",failure,total,
                (unsigned)valid,(unsigned)analysis.indexed,(unsigned)analysis.diagnostic.code);
        }
        assert(!valid && !analysis.indexed && analysis.diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY);
        destroy_editor();
    }
}
/** Missing selected source cannot expose a partial catalog or fabricated binding target. */
static void it_reports_missing_selected_sources_without_partial_semantic_queries(void) {
    setup_editor();
    ModuleRoot root = {"generated/absent",NULL,0,MODULE_ROOT_PROJECT};
    assert(!editor_analysis_build(&analysis,&root,1,".main") && analysis.state == EDITOR_UNAVAILABLE);
    assert(diagnostic_valid(&analysis.diagnostic));
    assert(!editor_definition(&analysis,".main",0,NULL));
    assert(!editor_highlights(&analysis,".main",tokens,NULL,NULL));
    destroy_editor();
}
/** Count exact structured warnings/errors while checking their borrowed original locations. */
static bool diagnostics(void *context, const Diagnostic *diagnostic, bool warning) {
    size_t *count = context;
    assert(diagnostic_valid(diagnostic));
    assert(warning && diagnostic->code == DIAGNOSTIC_EMPTY_BODY && text_is(diagnostic->primary.span,":"));
    (*count)++;
    return true;
}
/** A valid empty refusal block reports one parser warning, without duplicate fallback inspection records. */
static void it_exposes_structured_empty_block_warnings_without_turning_them_into_errors(void) {
    project_fixture("@!\nvalue\n.cell\nvalue !\n:\n");
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed);
    size_t count = 0;
    assert(editor_diagnostics(&analysis,diagnostics,&count) && count == 1);
    assert(point(analysis.selected->text,":") != 0);
    destroy_editor();
}
/** Run immutable ownership, exact failure enumeration and unavailable-source composition cases. */
void run(void) {
    it_exposes_structured_empty_block_warnings_without_turning_them_into_errors();
    it_keeps_old_definitions_valid_while_analyzing_changed_unicode_source();
    it_releases_temporary_highlight_failure_and_preserves_the_checked_snapshot();
    it_releases_every_failed_snapshot_and_occurrence_index_construction_prefix();
    it_reports_missing_selected_sources_without_partial_semantic_queries();
}
/** Every independent snapshot must release all counted semantic/source/index credits. */
int main(void) { run(); return EXIT_SUCCESS; }
