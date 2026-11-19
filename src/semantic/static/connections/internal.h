/** @file
 * @brief Share narrow runtime binding ownership and first-error operations.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial bindings.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) fixes source-order classification.
 */
#ifndef AMA_STATIC_CONNECTION_INTERNAL_H
#define AMA_STATIC_CONNECTION_INTERNAL_H
#include "plan.h"
#include "../constants/internal.h"
#include "../internal.h"

/** Allocate zeroed owned metadata while healthy; refusal is sticky.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) retains partial records.
 */
void *static_connection_allocate(StaticConnectionPlan *, size_t);
/** Retain only the first diagnostic without copying borrowed source storage.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves original provenance.
 */
bool static_connection_fail(StaticConnectionPlan *, StaticConnectionStatus, SourceSpan, const char *);
/** Propagate actual borrowed dependency errors, never synthesize a successful fallback.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains first errors.
 */
bool static_connection_dependencies(StaticConnectionPlan *);
/** Collect finite events from the original statement catalog without name lookup.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) preserves actual operand positions.
 */
bool static_runtime_events(StaticConnectionPlan *, StaticRuntimeModule *);
/** Stably order original source events without allocation or changing identities.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) fixes lexical processing order.
 */
void static_runtime_sort(StaticRuntimeModule *);
/** Insert a prepared source binding in source-site order; locals retain scope.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) fixes declaration identities.
 */
bool static_runtime_add_field(StaticConnectionPlan *, StaticRuntimeModule *, StaticRuntimeField, const StaticRuntimeField **);
/** Query module-wide storage only, excluding every lexical local.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) separates scope classes.
 */
const StaticRuntimeField *static_runtime_field(const StaticRuntimeModule *, SourceSpan);
/** Select the nearest exact enclosing foreach-body binding at this actual site.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) excludes headers/sibling bodies.
 */
const StaticRuntimeField *static_runtime_local(const StaticRuntimeModule *, SourceSpan, SourceSpan);
/** Retrieve an original explicit declaration's already collected field identity.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) hoists explicit lookup independently of encounters.
 */
const StaticRuntimeField *static_runtime_declaration(const StaticRuntimeModule *, const AstNode *);
/** Collect all explicit runtime declarations before processing any variable use.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) forbids late inferred shadowing.
 */
bool static_runtime_fields(StaticConnectionPlan *, StaticRuntimeModule *);
/** Reject duplicate module declarations with exact later/earlier source sites.
 * [NAME-04](../../../../spec/language/declarations.md#name-04) forbids shadowing.
 */
bool static_runtime_duplicate(StaticConnectionPlan *, const StaticRuntimeModule *, SourceSpan, SourceSpan);
/** Record one exact module-name encounter, leaving output untouched on refusal.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) fixes receiver freshness from the scanned prefix.
 */
bool static_runtime_encounter(StaticConnectionPlan *, StaticRuntimeModule *, SourceSpan, bool *);
/** Bind and retain one actual variable use, resolving lexical scope first.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) forbids construction-time local access.
 */
bool static_runtime_use(StaticConnectionPlan *, StaticRuntimeModule *, StaticRuntimeUse, const StaticRuntimeField **);
/** Visit an actual caller access and all index operands, checking known storage.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) keeps indices in the caller's namespace.
 */
bool static_runtime_access(StaticConnectionPlan *, StaticRuntimeModule *, const StaticRuntimeEvent *,
    const AstAccess *, bool, const StaticRuntimeField **);
/** Bind a source's actual names without deriving its concrete Type or value.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) excludes literals and qualified members.
 */
bool static_runtime_source(StaticConnectionPlan *, StaticRuntimeModule *, const StaticRuntimeEvent *, const AstSource *);
/** Classify and retain a simple/indexed assignment or output receiver exactly once.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) fixes fresh versus existing fields before solving.
 */
bool static_runtime_receiver(StaticConnectionPlan *, StaticRuntimeModule *, const StaticRuntimeEvent *,
    const AstAccess *, const StaticRuntimeReceiver **);
/** Resolve a foreach source and reuse/introduce its scalar reference at the header.
 * [FLOW-05](../../../../spec/language/lists.md#flow-05) forbids incompatible shadowing.
 */
bool static_runtime_foreach(StaticConnectionPlan *, StaticRuntimeModule *, const StaticRuntimeEvent *);
/** Consume each sorted source event once, without Type-dependent classification.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) separates name binding and equations.
 */
bool static_runtime_bind(StaticConnectionPlan *, StaticRuntimeModule *);
/** Validate an already published exact typed scope before borrowing its fields.
 * [TPL-04](../../../../spec/language/templates.md#tpl-04) preserves scope ownership.
 */
bool static_connection_has_scope(const StaticConnectionPlan *, const StaticTypedScope *);
/** Prepare source-owned terms for every immutable original runtime field.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) separates Type holes from source-visible static names.
 */
bool static_typed_fields(StaticConnectionPlan *, StaticTypedScope *);
/** Classify an original already-bound source using retained use identities.
 * Literal and qualified constant sources report Pending with operand untouched;
 * they need destination capability or independent namespace services. No name
 * lookup, ordinary body expansion or literal Type inference occurs here.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) fixes source storage and Type evidence.
 */
bool static_typed_operand(StaticConnectionPlan *, const StaticTypedScope *,
    const StaticRuntimeEvent *, const AstSource *, StaticTypedOperand *, StaticResultOutcome *);
/** Retain an original qualified source's independent namespace and exact
 * destination evidence. Literal sources require no constant record. Reuse exact
 * source/context/Type/form requests without treating metadata as validation.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) preserves independent namespace closure.
 */
bool static_typed_defer_constant(StaticConnectionPlan *, const StaticResultContext *,
    const AstSource *, const StaticTerm *, bool, bool, bool *);
/** Select a retained output/assignment receiver by original statement/access
 * identity and expose its Type plus exact scalar/whole-list storage form.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) forbids reconstructing receiver names after solving.
 */
bool static_typed_receiver_operand(StaticConnectionPlan *, const StaticTypedScope *,
    const AstNode *, const AstAccess *, StaticTypedOperand *);
#endif
