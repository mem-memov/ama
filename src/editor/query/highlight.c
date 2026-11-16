/** @file
 * @brief Enumerate original lexer categories without a recovery grammar or second whitespace policy.
 * [SYN-06](../../../spec/language/source-and-names.md#syn-06) fixes complete multiword tokens;
 * [SRC-03](../../../spec/language/source-and-names.md#src-03) preserves literal/comment bytes.
 */
#include "../analysis/analysis.h"
/** A temporary layout borrows the retained source; successful visits never outlive snapshot storage. Component contract: [SYN-06](../../../spec/language/source-and-names.md#syn-06), [SRC-03](../../../spec/language/source-and-names.md#src-03). */
bool editor_highlights(const EditorAnalysis *analysis, const char *module,
    EditorHighlightVisitor visit, void *context, Diagnostic *error) {
    const ModuleSource *source = NULL;
    if (!analysis->attempted || visit == NULL || module_catalog_find(&analysis->frontend.project.catalog, module, &source) != MODULE_OK) { return false; }
    SourceLayout layout;
    layout_init(&layout, analysis->frontend.allocator);
    bool valid = layout_read(&layout, source->text) == LAYOUT_OK;
    if (!valid && error != NULL) { *error = layout.diagnostic; }
    for (const LayoutLine *line = valid ? layout.first : NULL; line != NULL; line = line->next) {
        if (line->kind == LINE_BLANK) { continue; }
        if (line->kind == LINE_COMMENT) {
            EditorHighlight highlight = {.spelling=line->comment,.comment=true};
            if (!visit(context, &highlight)) { break; }
            continue;
        }
        LexCursor cursor;
        if (!lex_line_begin(&cursor, line, line->indentation)) {
            valid = false;
            if (error != NULL) { *error = cursor.error; }
            break;
        }
        LexToken token;
        LexStatus status;
        bool stopped = false;
        while ((status = lex_token_next(&cursor, &token)) == LEX_TOKEN) {
            EditorHighlight highlight = {token.text,token.payload,token.kind,false};
            if (!visit(context, &highlight)) { stopped = true; break; }
        }
        if (stopped) { break; }
        if (status == LEX_INVALID) {
            valid = false;
            if (error != NULL) { *error = cursor.error; }
            break;
        }
        if (line->comment.source != NULL) {
            EditorHighlight highlight = {.spelling=line->comment,.comment=true};
            if (!visit(context, &highlight)) { break; }
        }
    }
    layout_destroy(&layout);
    return valid;
}
