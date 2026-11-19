/** @file
 * @brief Separate attributed Type, operation and constant requirements from worklist orchestration.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains actual dependencies without false creation edges.
 */
#ifndef AMA_STATIC_CLOSURE_DEMAND_INTERNAL_H
#define AMA_STATIC_CLOSURE_DEMAND_INTERNAL_H
#include "../internal.h"

/** Require a complete ordinary Type under an original actual consumer. Pending
 * leaves the context receiver unchanged; success retains guard/registry provenance.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) validates supplied Types.
 */
bool static_closure_require(StaticClosurePlan *, const StaticResultContext *, const StaticTerm *,
    SourceSpan, const StaticResultContext **, bool *);
/** Require every declared/inferred/local field element Type, including empty storage.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) closes all original fields.
 */
bool static_closure_fields(StaticClosurePlan *, const StaticClosureEntry *, bool *);
/** Require all original calls and saved reads without runtime reachability filtering.
 * [TPL-06](../../../../../spec/language/templates.md#tpl-06) checks every operation.
 */
bool static_closure_operations(StaticClosurePlan *, const StaticClosureEntry *, bool *);
/** Resolve deferred direct constant transfers through independently known namespaces.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) forbids receiver-driven namespace choice.
 */
bool static_closure_constants(StaticClosurePlan *, bool *);
#endif
