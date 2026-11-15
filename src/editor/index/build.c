/** @file
 * @brief Index validated declarations and all flat resolved statements without reparsing names.
 * [NAME-06](../../../spec/language/declarations.md#name-06) preserves distinct lexical locals;
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08) binds interface labels exactly.
 */
#include "../analysis/internal.h"
/** Declaration names include inferred and foreach-local cells, while only explicit declarations have written type paths. */
static bool symbol(EditorAnalysis *analysis, const SemanticSymbol *field) {
    if (!editor_index_add(analysis, field->name, field, field->type, true)) { return false; }
    if (field->declaration != NULL
        && !editor_index_add(analysis, field->type_site, NULL, field->type, false)) { return false; }
    for (const SemanticInitializer *item = field->initializers; item != NULL; item = item->next) {
        if (!editor_index_source(analysis, item->source)) { return false; }
    }
    return true;
}
/** Binding syntax labels identify the callee field; source/receiver spellings identify caller storage. */
static bool binding(EditorAnalysis *analysis, const SemanticBinding *item) {
    if (item->field != NULL && item->syntax->label != NULL
        && !editor_index_add(analysis, item->syntax->label->name, item->field, item->field->type, false)) { return false; }
    return editor_index_source(analysis, item->source) && editor_index_access(analysis, item->receiver);
}
/** All nested operations already occur once in the semantic statement inventory; aliases require no recursive traversal. */
static bool statement(EditorAnalysis *analysis, const SemanticStatement *item) {
    if (!editor_index_access(analysis, item->access) || !editor_index_access(analysis, item->condition.access)
        || !editor_index_source(analysis, item->source)) { return false; }
    if (item->syntax->kind == AST_CALL
        && !editor_index_add(analysis, item->syntax->as.call.namespace_path, NULL, item->namespace_type, false)) { return false; }
    for (const SemanticBinding *argument = item->bindings; argument != NULL; argument = argument->next) {
        if (!binding(analysis, argument)) { return false; }
    }
    if (item->traversal != NULL) {
        const SemanticForeachBinding *loop = item->traversal;
        if (!editor_index_add(analysis, loop->syntax->as.traversal.collection,
                loop->collection, loop->collection->type, false)
            || !editor_index_add(analysis, loop->syntax->as.traversal.binder,
                loop->binding, loop->binding->type, loop->introduced)) { return false; }
    }
    return true;
}
/** Publish no metadata until every contributing type, field and operand has been indexed and ordered consistently. */
bool editor_index_build(EditorAnalysis *analysis) {
    for (const SemanticType *type = analysis->frontend.semantic.first; type != NULL; type = type->next) {
        for (const SemanticSymbol *field = type->first; field != NULL; field = field->next) {
            if (!symbol(analysis, field)) { return false; }
        }
        for (const SemanticSymbol *local = type->locals; local != NULL; local = local->next) {
            if (!symbol(analysis, local)) { return false; }
        }
        for (const SemanticStatement *item = type->statements; item != NULL; item = item->next) {
            if (!statement(analysis, item)) { return false; }
        }
    }
    return editor_index_order(analysis);
}
