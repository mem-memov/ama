/**
 * @file
 * @brief Identify syntactic interface introduction candidates before category resolution.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) and
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) first classify
 * candidate introductions versus receivers in lexical order. Output actions do
 * not disqualify an otherwise bare interface head or supply input slot names.
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) checks known constructor patterns.
 */
#ifndef AMA_STATIC_CLASSIFY_SITES_H
#define AMA_STATIC_CLASSIFY_SITES_H
#include "../internal.h"

/** Return Type/Template for a syntactically eligible candidate, deferred for
 * a noncandidate. This predicate does not resolve local scope or actual types.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
StaticCategory static_introduction_candidate(const StaticOccurrence *);

/** Recognize a root runtime input with named static inputs, independently of
 * whether its head is concrete, explicit or implicitly introduced.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
bool static_named_input_pattern(const StaticOccurrence *);

/** Collect implicit owner parameters and aliases after explicit declarations.
 * Input occurrences must already be sorted by source offset. Template category
 * is collected over the whole interface before considering remaining bare heads.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool static_classify_names(StaticInventory *, StaticModule *);
#endif
