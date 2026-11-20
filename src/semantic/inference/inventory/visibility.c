/** @file
 * @brief Classify typed creation-time constant and declaration-order availability.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) separates module-wide,
 * source-ordered inferred and body-scoped foreach names; constructor inputs use declaration order.
 */
#include "../internal.h"
/** Pure counterpart of constructor diagnostics; callers can inspect without mutating semantic state. Component contract: [NAME-06](../../../../spec/language/declarations.md#name-06). */
SemanticVisibility semantic_initializer_visibility(const SemanticSymbol *symbol, const SemanticSymbol *initializing) {
    if (initializing == NULL) { return SEMANTIC_VISIBLE; }
    if (initializing->role == SYMBOL_CONSTANT && symbol->role != SYMBOL_CONSTANT) { return SEMANTIC_CONSTANT_ONLY; }
    if (symbol->owner == initializing->owner && symbol->ordinal >= initializing->ordinal) { return SEMANTIC_NOT_INITIALIZED; }
    return SEMANTIC_VISIBLE;
}
