/** @file
 * @brief Map original byte boundaries through checked nonoverlapping complete-name replacements.
 * [SRC-03](../../../../spec/language/source-and-names.md#src-03) preserves every intervening byte;
 * [ARCH-06](../../../../spec/implementation/compiler.md#arch-06) requires exact coordinate proofs.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"
/** Find the last edit starting at/before a boundary; stored new ends already include all preceding deltas. */
static bool boundary(const EditorRenameFile *file, size_t old, size_t *output) {
    if (file == NULL) { *output = old; return true; }
    size_t low = 0, high = file->edit_count;
    while (low < high) {
        size_t middle = low+(high-low)/2;
        if (file->edits[middle]->original.start <= old) { low = middle+1; }
        else { high = middle; }
    }
    if (low == 0) { *output = old; return true; }
    const EditorRenameEdit *edit = file->edits[low-1];
    if (old == edit->original.start) { *output = edit->new_start; return true; }
    if (old < edit->original.end) { return false; }
    size_t suffix = old-edit->original.end;
    if (suffix > SIZE_MAX-edit->new_end) { return false; }
    *output = edit->new_end+suffix;
    return true;
}
/** Binary lookup uses physical path plus exact retained source identity; no shorter-name or source fallback is accepted. */
bool editor_rename_span(const EditorRename *plan, SourceSpan old, SourceSpan *output) {
    if (old.source == NULL) {
        if (old.start != 0 || old.end != 0) { return false; }
        *output = (SourceSpan){0};
        return true;
    }
    if (!source_span_valid(old)) { return false; }
    size_t low = 0, high = plan->source_count;
    const EditorRenameSource *pair = NULL;
    while (low < high) {
        size_t middle = low+(high-low)/2;
        const EditorRenameSource *item = &plan->source_index[middle];
        int order = strcmp(source_path(old.source),source_path(item->original->text));
        if (order == 0) { pair = item; break; }
        if (order < 0) { high = middle; } else { low = middle+1; }
    }
    if (pair == NULL || pair->original->text != old.source) { return false; }
    SourceSpan mapped = {.source=pair->candidate->text};
    if (!boundary(pair->file,old.start,&mapped.start) || !boundary(pair->file,old.end,&mapped.end)
        || !source_span_valid(mapped)) { return false; }
    *output = mapped;
    return true;
}
