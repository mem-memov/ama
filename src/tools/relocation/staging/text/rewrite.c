/** @file
 * @brief Rebuild one text from exact nonoverlapping path spans and unchanged intervening bytes.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * makes line endings, comments and all unselected bytes invariant under rewriting.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"
/** Sum retained segments and replacement bytes with checked additions before allocating the output. Component contract: [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03). */
bool relocation_staging_rewrite(RelocationStaging *staging, const SourceText *original_text, const char *path,
    const RelocationPathEdit *const *edits, size_t edit_count, const SourceText **result) {
    size_t length = 0, input = 0;
    for (size_t index = 0; index < edit_count; index++) {
        const RelocationPathEdit *edit = edits[index];
        if (!source_span_valid(edit->original) || edit->original.start == edit->original.end
            || edit->original.end > source_length(original_text)
            || edit->original.start < input) {
            return relocation_staging_fail(staging,RELOCATION_REFERENCE_CONFLICT,"Prepared source edits overlap",edit->original);
        }
        size_t retained = edit->original.start-input, replacement = strlen(edit->replacement);
        if (retained > SIZE_MAX-length || replacement > SIZE_MAX-length-retained) {
            return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Prepared source size is unrepresentable",edit->original);
        }
        length += retained+replacement;
        input = edit->original.end;
    }
    size_t tail = source_length(original_text)-input;
    if (tail > SIZE_MAX-length) {
        return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Prepared source size is unrepresentable",(SourceSpan){0});
    }
    length += tail;
    SourceArena scratch;
    source_arena_init(&scratch,staging->arena.allocator);
    unsigned char *bytes = source_arena_allocate(&scratch,length);
    if (bytes == NULL) {
        return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Prepared source byte allocation failed",(SourceSpan){0});
    }
    input = 0;
    size_t output = 0;
    const unsigned char *original = source_bytes(original_text);
    for (size_t index = 0; index < edit_count; index++) {
        const RelocationPathEdit *edit = edits[index];
        size_t retained = edit->original.start-input, replacement = strlen(edit->replacement);
        memcpy(bytes+output,original+input,retained);
        output += retained;
        memcpy(bytes+output,edit->replacement,replacement);
        output += replacement;
        input = edit->original.end;
    }
    memcpy(bytes+output,original+input,tail);
    SourceStatus copied = source_manager_add(&staging->sources,path,bytes,length,result);
    source_arena_destroy(&scratch);
    if (copied != SOURCE_OK) {
        return relocation_staging_fail(staging,RELOCATION_OUT_OF_MEMORY,"Prepared source ownership allocation failed",(SourceSpan){0});
    }
    return true;
}
