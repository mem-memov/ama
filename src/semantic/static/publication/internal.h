/** @file
 * @brief Share checked publication allocation, closure and original receiver evidence operations.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates evidence and publication.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial descriptors.
 */
#ifndef AMA_STATIC_PUBLICATION_INTERNAL_H
#define AMA_STATIC_PUBLICATION_INTERNAL_H
#include "plan.h"
#include "../results/internal.h"

/** Preserve the first diagnostic without allocating a failure report.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains original locations.
 */
bool static_publication_fail(StaticPublicationPlan *, StaticPublicationStatus, SourceSpan, const char *);
/** Propagate first borrowed errors; healthy Pending is not a failure.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves cause.
 */
bool static_publication_dependencies(StaticPublicationPlan *);
/** Allocate zeroed plan-owned storage or record resource refusal.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial records.
 */
void *static_publication_allocate(StaticPublicationPlan *, size_t);
/** Locate the original lexical receiver symbol by exact expression/binding.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) prevents name capture.
 */
const StaticSymbol *static_publication_receiver(const StaticResultUse *, const AstStaticBinding *);
/** Close the exact reachable source proof graph, including existing receiver
 * definitions as independent roots rather than invented definition-cycle edges.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates dependency graphs.
 */
bool static_publication_dependencies_closed(StaticPublicationPlan *, StaticResultUse *, bool *);
/** Refine only while genuine equation/metadata/requirement or checked context evidence changes;
 * require closed source dependencies before returning a canonical value.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) forbids guessed inverses.
 */
bool static_publication_close(StaticPublicationPlan *, StaticResultUse *, const StaticTerm **);
/** Construct ordered closed outputs and immutable aliases before exposing the
 * source publication. Failure keeps partial descriptors owned.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) requires complete results.
 */
bool static_publication_outputs(StaticPublicationPlan *, StaticPublication *);
/** Intern one closed alias with exact lexical identity and canonical equality.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves immutable aliases.
 */
bool static_publication_alias(StaticPublicationPlan *, const StaticTermEnvironment *, const StaticSymbol *, const StaticTerm *);
#endif
