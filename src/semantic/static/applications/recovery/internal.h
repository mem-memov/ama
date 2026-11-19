/** @file
 * @brief Separate named-set validation from B-minus-P constructor recovery.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) requires a unique
 * factorization from an actual nominal result, without expected-signature search.
 */
#ifndef AMA_STATIC_RECOVERY_INTERNAL_H
#define AMA_STATIC_RECOVERY_INTERNAL_H
#include "../internal.h"
/** Find an explicitly supplied whole name in the owned source request chain.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves named keys.
 */
const BindingRequest *static_recovery_supplied(const StaticTermApplication *, const StaticSymbol *);
/** Require every applied name to be an original formal fixed in the known result.
 * Duplicate/source-kind checks precede this phase in request normalization.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) proves P is a subset of dom(B).
 */
bool static_recovery_names(StaticApplicationPlan *, const StaticTermApplication *, const StaticTerm *);
/** Equate the unknown head with the only candidate retaining B outside P.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves S union P,
 * and the normal forward result equation validates every removed binding value.
 */
bool static_recovery_constructor(StaticApplicationPlan *, const StaticTermApplication *,
    const StaticTerm *, const StaticTerm *, SourceSpan);
#endif
