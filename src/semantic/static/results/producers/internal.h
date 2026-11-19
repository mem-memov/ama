/** @file
 * @brief Separate full producer input-map construction from named request validation and publication.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves omitted formal identities.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
#ifndef AMA_STATIC_RESULT_PRODUCERS_INTERNAL_H
#define AMA_STATIC_RESULT_PRODUCERS_INTERNAL_H
#include "../internal.h"

/** The caller supplies an owned producer source and actual-origin normalized
 * requests. Build a full original-formal-order array, retaining explicit child
 * value identities and stable holes for missing inputs. No child body, guard or
 * solver runs. An empty map succeeds with null; refusal leaves output untouched.
 * Partial allocations stay with their result/term owners.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) distinguishes omission from a free input.
 */
bool static_result_producer_inputs(StaticResultPlan *, const StaticResultSource *,
    const BindingApplication *, const StaticTermCapture **);
#endif
