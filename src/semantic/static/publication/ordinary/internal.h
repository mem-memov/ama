/** @file
 * @brief Share exact ordinary-Type membership and independent export resolution.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) keeps ordinary source identity exact.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) isolates producer inference.
 */
#ifndef AMA_STATIC_ORDINARY_INTERNAL_H
#define AMA_STATIC_ORDINARY_INTERNAL_H
#include "../types.h"
#include "../internal.h"

/** Validate registered pointer identity before accessing a supplied record.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) preserves the original consumer.
 */
bool static_ordinary_has(const StaticPublicationPlan *, const StaticOrdinaryType *);
/** Close the actual independent producer root, select a public Type and only
 * then equate it with the consumer-local variable. Retain the validation edge.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) forbids contextual completion.
 */
bool static_ordinary_export(StaticPublicationPlan *, StaticOrdinaryType *);
#endif
