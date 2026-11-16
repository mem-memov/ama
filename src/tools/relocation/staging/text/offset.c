/** @file
 * @brief Translate original edge boundaries through exact nonoverlapping replacements.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves graph edges while [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * leaves all intervening bytes unchanged.
 */
#include <string.h>
#include "../internal.h"
/** Parallel input/output cursors avoid signed deltas and reject boundaries inside replaced path text. Component contract: [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02), [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03). */
bool relocation_staging_offset(const SourceText *source, const RelocationPathEdit *const *edits, size_t edit_count, size_t original, size_t *mapped) {
    if (original > source_length(source)) { return false; }
    size_t input = 0, output = 0;
    for (size_t index = 0; index < edit_count; index++) {
        const RelocationPathEdit *edit = edits[index];
        if (original <= edit->original.start) { break; }
        if (original < edit->original.end) { return false; }
        output += edit->original.start-input;
        output += strlen(edit->replacement);
        input = edit->original.end;
    }
    *mapped = output+(original-input);
    return true;
}
