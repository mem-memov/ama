/**
 * @file
 * @brief Recognize eligible bare and named input-pattern heads without resolving names.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) limits implicit
 * introductions to interface expressions. Output-only heads remain bare;
 * only root input patterns with named input labels introduce Template.
 */
#include "sites.h"

/** Count directions independently: skips are inputs too, output labels never
 * describe a constructor's expected input signature. A positional-only unknown
 * constructor is not an implicit Template introduction.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
StaticCategory static_introduction_candidate(const StaticOccurrence *occurrence) {
    if (occurrence->receiver != NULL || occurrence->expression->kind != AST_TYPE_PLACEHOLDER) {
        return STATIC_CATEGORY_DEFERRED;
    }
    if (occurrence->context != STATIC_CONTEXT_INPUT && occurrence->context != STATIC_CONTEXT_RESULT
        && occurrence->context != STATIC_CONTEXT_DEFINITION) { return STATIC_CATEGORY_DEFERRED; }
    bool inputs = false, named = false;
    for (const AstStaticBinding *binding = occurrence->expression->first; binding != NULL; binding = binding->next) {
        if (binding->kind != AST_STATIC_INPUT) { continue; }
        inputs = true;
        if (!binding->positional) { named = true; }
    }
    if (!inputs) { return STATIC_CATEGORY_TYPE; }
    return occurrence->context == STATIC_CONTEXT_INPUT && occurrence->root && named
        ? STATIC_CATEGORY_TEMPLATE : STATIC_CATEGORY_DEFERRED;
}

/** A concrete or previously declared constructor also has a named input pattern;
 * the foreign names must be checked without introducing a local head.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
bool static_named_input_pattern(const StaticOccurrence *occurrence) {
    if (occurrence->receiver != NULL || occurrence->context != STATIC_CONTEXT_INPUT || !occurrence->root) { return false; }
    for (const AstStaticBinding *binding = occurrence->expression->first; binding != NULL; binding = binding->next) {
        if (binding->kind == AST_STATIC_INPUT && !binding->positional) { return true; }
    }
    return false;
}
