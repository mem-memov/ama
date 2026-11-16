/** @file
 * @brief Validate the complete replacement using production UTF-8/layout and complete-name scanning.
 * [SYN-06](../../../../spec/language/source-and-names.md#syn-06) admits one exact name;
 * [NAME-04](../../../../spec/language/declarations.md#name-04) forbids storage suffix overloading.
 */
#include "../internal.h"
/** Name validation never trims, normalizes, appends a suffix or interprets embedded punctuation as code. Component contract: [SYN-06](../../../../spec/language/source-and-names.md#syn-06), [NAME-04](../../../../spec/language/declarations.md#name-04). */
bool editor_rename_name(EditorRename *plan, const unsigned char *bytes, size_t length) {
    const SourceText *source = NULL;
    SourceStatus status = source_manager_add(&plan->sources,"<rename>",bytes,length,&source);
    if (status == SOURCE_INVALID_ARGUMENT) {
        return editor_rename_fail(plan,DIAGNOSTIC_SYNTAX,"Replacement name bytes are absent",(SourceSpan){0});
    }
    if (status != SOURCE_OK) {
        return editor_rename_fail(plan,DIAGNOSTIC_OUT_OF_MEMORY,"Replacement name allocation failed",(SourceSpan){0});
    }
    SourceLayout layout;
    layout_init(&layout,plan->arena.allocator);
    bool valid = layout_read(&layout,source) == LAYOUT_OK;
    if (!valid) { plan->diagnostic = layout.diagnostic; }
    layout_destroy(&layout);
    if (!valid) { return false; }
    SourceSpan full = {source,0,length}, name = {0};
    size_t cursor = 0;
    if (!lex_name(full,&cursor,&name) || cursor != length || name.start != 0 || name.end != length) {
        return editor_rename_fail(plan,DIAGNOSTIC_SYNTAX,"Replacement must be one complete Ama base name",full);
    }
    plan->name = name;
    return true;
}
