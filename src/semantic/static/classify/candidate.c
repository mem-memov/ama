/**
 * @file
 * @brief Recognize ordinary interface Type heads without resolving names.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) limits ordinary
 * introductions to bare input/output Type lines; graph requirements are separate.
 */
#include "sites.h"

/** Only a bare ordinary interface Type can introduce an ordinary formal.
 * Graph free-input requirements are collected by the separate graph scope phase.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
StaticCategory static_introduction_candidate(const StaticOccurrence *occurrence) {
    if (occurrence->receiver != NULL || occurrence->expression->kind != AST_TYPE_PLACEHOLDER
        || occurrence->expression->first != NULL) { return STATIC_CATEGORY_DEFERRED; }
    return occurrence->context == STATIC_CONTEXT_INPUT || occurrence->context == STATIC_CONTEXT_RESULT
        ? STATIC_CATEGORY_TYPE : STATIC_CATEGORY_DEFERRED;
}
