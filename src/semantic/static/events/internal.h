/**
 * @file
 * @brief Declare structural static occurrence construction without name classification.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) needs source-context
 * information before introduction classification; [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * requires original output receiver positions for alias visibility.
 */
#ifndef AMA_STATIC_EVENTS_INTERNAL_H
#define AMA_STATIC_EVENTS_INTERNAL_H
#include "../internal.h"

/** Append one owned occurrence after count/allocation checks. No symbol or
 * producer origin is inferred. Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
bool static_occurrence_append(StaticInventory *, StaticModule *, StaticOccurrence);
/** Enumerate one optional expression's owned heads/receivers with explicit
 * frames. Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * excludes foreign labels and omitted output receivers from local names.
 */
bool static_expression_occurrences(StaticInventory *, StaticModule *,
    const AstTypeExpression *, const AstNode *, StaticContext);
#endif
