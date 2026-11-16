/**
 * @file
 * @brief Separate selected-child admission and operation completion from iterative call-frame dispatch.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) requires
 * source-ordered trees, exact handler ownership and unbounded finite parser depth.
 */
#ifndef AMA_PARSER_CALL_TREE_INTERNAL_H
#define AMA_PARSER_CALL_TREE_INTERNAL_H
#include "../../internal.h"
/** Admit one selected-result child and push its explicit argument frame; no lexical variable or temporary is declared. Component contract: [CALL-09](../../../../spec/language/calls-and-results.md#call-09). */
bool parser_selected_call(Parser *, ParseFrame **);
/** Finish an operation's output requirements and optional same-level refusal body, or pop its completed frame. Component contract: [CALL-09](../../../../spec/language/calls-and-results.md#call-09). */
bool parser_call_finish(Parser *, ParseFrame **);
#endif
