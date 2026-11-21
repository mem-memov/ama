/** @file
 * @brief Share exact ordinary-Type membership and independent export resolution.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) keeps ordinary source identity exact.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) keeps complete producer validation separate from value constraints.
 */
#ifndef AMA_STATIC_ORDINARY_INTERNAL_H
#define AMA_STATIC_ORDINARY_INTERNAL_H
#include "../types.h"
#include "../internal.h"

/** Validate registered pointer identity before accessing a supplied record.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) preserves the original consumer.
 */
bool static_ordinary_has(const StaticPublicationPlan *, const StaticOrdinaryType *);
/** Attach this actual selector without allocating, changing the primary cause,
 * or adding constraints/creation edges from the diagnostic itself.
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
