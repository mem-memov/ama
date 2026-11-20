/** @file
 * @brief Select a concrete editor context only when one actual Type exists for the original source module.
 * [TPL-05](../../../spec/language/templates.md#tpl-05) keeps unequal capture maps distinct.
 */
#ifndef AMA_EDITOR_QUERY_CONTEXT_H
#define AMA_EDITOR_QUERY_CONTEXT_H
#include "../analysis/analysis.h"
/** Borrow a unique concrete context from an already checked frontend inventory.
 * Zero or multiple contexts return null; no ordering chooses a specialization.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) exposes actual compiler metadata without guessing a missing context.
 */
const SemanticType *editor_query_type(const EditorAnalysis *, const char *);
#endif
