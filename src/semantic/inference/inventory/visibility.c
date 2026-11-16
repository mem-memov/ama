/** @file
 * @brief Expose exact lexical name availability from the completed declaration inventory.
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
/** Completed inventories retain all forward explicit declarations but cannot make a future inferred field visible retroactively. Component contract: [NAME-06](../../../../spec/language/declarations.md#name-06). */
bool semantic_symbol_available(const SemanticType *owner, const SemanticSymbol *symbol, SourceSpan site,
    const SemanticSymbol *initializing) {
    if (symbol->owner != owner || site.source != owner->module->source->text || !source_span_valid(site)) { return false; }
    const SemanticSymbol *local = semantic_local_at(owner, symbol->name, site);
    if (symbol->local) { return initializing == NULL && local == symbol; }
    if (local != NULL || (symbol->inferred && site.start < symbol->name.start)) { return false; }
    return semantic_initializer_visibility(symbol, initializing) == SEMANTIC_VISIBLE;
}
