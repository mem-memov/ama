/** @file
 * @brief Preserve exact foreach source and cell identities in each concrete owner.
 * [FLOW-05](../../../../spec/language/lists.md#flow-05) distinguishes new local and reused cells.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) preserves fixed field pointers.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) retains original lexical scope.
 */
#include "internal.h"

/** Search original pointer identities within one concrete owner, including locals.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) forbids rebinding by spelling.
 */
static SemanticSymbol *symbol(SemanticType *owner, const StaticRuntimeField *original) {
    for (SemanticSymbol *item = owner->first; item; item = item->next) {
        if (item->original == original) { return item; }
    }
    for (SemanticSymbol *item = owner->locals; item; item = item->next) {
        if (item->original == original) { return item; }
    }
    return NULL;
}

bool semantic_specialization_foreach(SemanticProgram *program, SemanticType *type) {
    for (const StaticRuntimeForeach *source = type->specialization->scope->runtime->first_foreach;
        source; source = source->next) {
        SemanticForeachBinding *binding = semantic_allocate(program, sizeof(*binding));
        if (!binding) { return false; }
        *binding = (SemanticForeachBinding){.syntax = source->syntax,
            .collection = symbol(type, source->collection), .binding = symbol(type, source->binding),
            .introduced = source->introduced};
        if (!binding->collection || !binding->binding) {
            return semantic_fail(program, source->syntax->span, "Traversal field is missing from its concrete owner", NULL);
        }
        if (type->last_foreach_binding) { type->last_foreach_binding->next = binding; }
        else { type->foreach_bindings = binding; }
        type->last_foreach_binding = binding;
    }
    return true;
}
