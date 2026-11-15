/** @file
 * @brief Enumerate exact lexical names and declared interfaces from compiler-owned semantic metadata.
 * [NAME-06](../../../spec/language/declarations.md#name-06) delegates visibility to semantic inventory predicates;
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08) supplies independent interface roles.
 */
#include "../analysis/analysis.h"
/** A point inside an actual initializer source uses constructor availability, not runtime foreach visibility. */
static const SemanticSymbol *initializing_at(const SemanticType *type, size_t offset) {
    for (const SemanticSymbol *field = type->first; field != NULL; field = field->next) {
        for (const SemanticInitializer *item = field->initializers; item != NULL; item = item->next) {
            SourceSpan span = item->source->site;
            if (span.source == type->module->source->text && offset >= span.start && offset < span.end) { return field; }
        }
    }
    return NULL;
}
/** Callbacks may stop enumeration but cannot invalidate snapshot ownership during the call. */
bool editor_completions(const EditorAnalysis *analysis, const char *module, size_t offset,
    EditorSymbolVisitor visit, void *context) {
    if (!analysis->indexed || visit == NULL) { return false; }
    const SemanticType *type = semantic_find_type(&analysis->frontend.semantic, module);
    if (type == NULL || offset > source_length(type->module->source->text)) { return false; }
    SourceSpan site = {type->module->source->text,offset,offset};
    const SemanticSymbol *initializing = initializing_at(type, offset);
    for (const SemanticSymbol *field = type->first; field != NULL; field = field->next) {
        if (semantic_symbol_available(type, field, site, initializing) && !visit(context, field)) { return true; }
    }
    for (const SemanticSymbol *local = type->locals; local != NULL; local = local->next) {
        if (semantic_symbol_available(type, local, site, initializing) && !visit(context, local)) { return true; }
    }
    return true;
}
/** Signature enumeration is role-specific and cannot expose private state as an external parameter/result. */
bool editor_signature(const EditorAnalysis *analysis, const char *module, SymbolRole role,
    EditorSymbolVisitor visit, void *context) {
    if (!analysis->indexed || visit == NULL || (role != SYMBOL_PARAMETER && role != SYMBOL_RESULT && role != SYMBOL_CONSTANT)) { return false; }
    const SemanticType *type = semantic_find_type(&analysis->frontend.semantic, module);
    if (type == NULL) { return false; }
    for (const SemanticSymbol *field = type->first; field != NULL; field = field->next) {
        if (field->role == role && !visit(context, field)) { break; }
    }
    return true;
}
