/**
 * @file
 * @brief Resolve each traversal header before entering its body and reuse exact compatible references without shadowing.
 * [FLOW-05](../../../../spec/language/lists.md#flow-05) fixes source/binding
 * lookup order; [NAME-06](../../../../spec/language/declarations.md#name-06)
 * keeps introduced cells local and persistent fields module-wide.
 */
#include "../internal.h"
bool inference_foreach(Inference *inference, const AstNode *node) {
    SourceSpan collection = node->as.traversal.collection;
    SemanticSymbol *source_symbol = inference_name(inference, collection, collection, false);
    if (source_symbol == NULL) { return false; }
    if (source_symbol->role == SYMBOL_CONSTANT || !source_symbol->list
        || source_symbol->reference != node->as.traversal.reference_elements) {
        return semantic_fail(inference->program, collection, "A traversal requires matching mutable list storage", source_symbol);
    }
    InferenceValue source = {.type = source_symbol->type, .symbol = source_symbol,
        .reference = source_symbol->reference, .list = true};
    SourceSpan name = node->as.traversal.binder;
    SemanticSymbol *binding = semantic_local_at(inference->type, name, name);
    if (binding == NULL) { binding = semantic_symbol(inference->type, name); }
    bool introduced = binding == NULL;
    if (binding != NULL) {
        if (binding->role == SYMBOL_CONSTANT || !binding->reference || binding->list || binding->type != source.type) {
            return semantic_fail(inference->program, name, "A traversal binding cannot shadow incompatible visible storage", binding);
        }
        if (!binding->local && !inference_encounter(inference, name, NULL)) { return false; }
    } else {
        binding = semantic_allocate(inference->program, sizeof(*binding));
        if (binding == NULL) { return false; }
        SourceSpan spelling = name;
        spelling.end++;
        *binding = (SemanticSymbol){.next = inference->type->locals, .name = name,
            .spelling = spelling, .site = spelling, .type_site = collection,
            .owner = inference->type, .type = source.type, .role = SYMBOL_STATE,
            .reference = true, .local = true, .scope = node};
        inference->type->locals = binding;
    }
    SemanticForeachBinding *role = semantic_allocate(inference->program, sizeof(*role));
    if (role == NULL) { return false; }
    *role = (SemanticForeachBinding){.syntax = node, .collection = source.symbol,
        .binding = binding, .introduced = introduced};
    if (inference->type->last_foreach_binding == NULL) { inference->type->foreach_bindings = role; }
    else { inference->type->last_foreach_binding->next = role; }
    inference->type->last_foreach_binding = role;
    return true;
}
