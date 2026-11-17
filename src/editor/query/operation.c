/** @file
 * @brief Expose exact nominal or intrinsic call metadata at its written target without fabricating declarations.
 * [LIST-04](../../../spec/language/lists.md#list-04) keeps intrinsic interfaces separate from module instances;
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) supplies checked IR directly.
 */
#include "../analysis/analysis.h"
/** Only the written target/path interval identifies this call; enclosing binding/body ranges are not a guess. Component contract: [LIST-04](../../../spec/language/lists.md#list-04), [COMP-02](../../../spec/implementation/compiler.md#comp-02). */
bool editor_operation(const EditorAnalysis *analysis, const char *module, size_t offset,
    const SemanticStatement **output) {
    if (!analysis->indexed || output == NULL) { return false; }
    const SemanticType *type = semantic_find_type(&analysis->frontend.semantic,module);
    if (type == NULL || offset >= source_length(type->module->source->text)) { return false; }
    for (const SemanticStatement *item = type->statements; item != NULL; item = item->next) {
        if (item->syntax->kind != AST_CALL) { continue; }
        SourceSpan span = item->syntax->as.call.mode == AST_CONSTANT_READ ? item->syntax->as.call.namespace_expression->head
            : item->syntax->as.call.target->span;
        if (span.source == type->module->source->text && offset >= span.start && offset < span.end) {
            *output = item;
            return true;
        }
    }
    return false;
}

/** A positional marker or written label identifies one checked interface binding, including intrinsic fields without source declarations. Component contract: [LIST-04](../../../spec/language/lists.md#list-04), [COMP-02](../../../spec/implementation/compiler.md#comp-02). */
bool editor_binding(const EditorAnalysis *analysis, const char *module, size_t offset,
    const SemanticBinding **output) {
    if (!analysis->indexed || output == NULL) { return false; }
    const SemanticType *type = semantic_find_type(&analysis->frontend.semantic,module);
    if (type == NULL || offset >= source_length(type->module->source->text)) { return false; }
    for (const SemanticStatement *item = type->statements; item != NULL; item = item->next) {
        for (const SemanticBinding *binding = item->bindings; binding != NULL; binding = binding->next) {
            SourceSpan label = binding->syntax->label == NULL ? binding->syntax->marker : binding->syntax->label->span;
            if (label.source == type->module->source->text && offset >= label.start && offset < label.end) {
                *output = binding;
                return true;
            }
        }
    }
    return false;
}
