/**
 * @file
 * @brief Identify syntactic interface introduction candidates before category resolution.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) and
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separate ordinary Type
 * interfaces from public graph definitions. [TPL-07](../../../../spec/language/templates.md#tpl-07)
 * leaves graph requirements to their independent collection stage.
 */
#ifndef AMA_STATIC_CLASSIFY_SITES_H
#define AMA_STATIC_CLASSIFY_SITES_H
#include "../internal.h"

/** Return Type for a syntactically eligible candidate, deferred for
 * a noncandidate. This predicate does not resolve local scope or actual types.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
StaticCategory static_introduction_candidate(const StaticOccurrence *);

/** Collect implicit owner parameters and aliases after explicit declarations.
 * Input occurrences must already be sorted by source offset. Ordinary parameters
 * come only from input/output Type lines, never a standalone formal declaration.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02),
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool static_classify_names(StaticInventory *, StaticModule *);
#endif
