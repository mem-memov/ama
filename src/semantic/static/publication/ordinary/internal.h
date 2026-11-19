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
/** Attach this actual selector without allocating, changing the primary cause,
 * or turning its caller into a producer inference/creation input.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) preserves requiring-site provenance.
 */
void static_ordinary_error_context(StaticPublicationPlan *, StaticOrdinaryType *);
/** Reject a selected non-Type with its actual export definition and an original
 * residual witness, preserving all prior error state and complete metadata.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) links responsible original declarations.
 */
bool static_ordinary_export_failure(StaticPublicationPlan *, StaticOrdinaryType *, StaticPublicationStatus,
    const char *, const StaticTerm *);
#endif
