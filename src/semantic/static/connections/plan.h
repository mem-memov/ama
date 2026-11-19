/** @file
 * @brief Own lexical runtime bindings and typed-connection work separately from source and static values.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) fixes bindings before solving.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates scratch ownership.
 */
#ifndef AMA_STATIC_CONNECTION_PLAN_H
#define AMA_STATIC_CONNECTION_PLAN_H
#include "../constants/plan.h"
#include "runtime/model.h"
#include "typed/model.h"

/** Distinguish malformed identities, semantic conflicts, unresolved work and
 * resource refusal; a failed owner is destructible only.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) preserves unknown work without guessing.
 */
typedef enum { STATIC_CONNECTION_OK, STATIC_CONNECTION_INVALID, STATIC_CONNECTION_CONFLICT,
    STATIC_CONNECTION_UNRESOLVED, STATIC_CONNECTION_OUT_OF_MEMORY } StaticConnectionStatus;

/** A deferred qualified constant transfer retains its original source and exact
 * destination Type/form. The namespace is independently fixed; destination
 * evidence must never determine it. Values and native resources are not read.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) separates namespace closure from member transfer.
 */
typedef struct StaticTypedConstant {
    struct StaticTypedConstant *next;
    const StaticConstantSource *source;
    const StaticTerm *type;
    bool equations_ready;
    bool reference;
    bool list;
} StaticTypedConstant;

/** Destroy before borrowed constants, publication/term/inventory/source owners.
 * All lexical records borrow original syntax, never runtime values.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds failed preparation.
 */
typedef struct {
    SourceArena arena;
    StaticConstantPlan *constants;
    StaticRuntimeModule *first_runtime;
    size_t runtime_count;
    StaticTypedScope *first_scope;
    size_t scope_count;
    StaticTypedContext *first_context;
    size_t context_count;
    StaticTypedOperation *first_operation;
    size_t operation_count;
    StaticTypedConstant *first_constant;
    size_t constant_count;
    StaticConnectionStatus status;
    Diagnostic error;
    DiagnosticRelated related;
} StaticConnectionPlan;

/** Borrow healthy dependencies without allocation or source/Type evaluation.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates owners.
 */
void static_connection_init(StaticConnectionPlan *, StaticConstantPlan *, SourceAllocator);
/** Release complete/partial records without following borrowed syntax/value links.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds teardown.
 */
void static_connection_destroy(StaticConnectionPlan *);
/** Classify one original module's names exactly once independently of Type values.
 * Reuse existing immutable bindings. Failure keeps output untouched and all partial
 * work owned. No operation body executes or callee specialization is guessed.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) defines lexical classification.
 */
bool static_connection_runtime(StaticConnectionPlan *, const StaticModule *, const StaticRuntimeModule **);
/** Prepare source-owned Type terms for the original fields of one exact nominal
 * environment. A generic environment needs an already registered actual root;
 * a supplied nominal environment is interface metadata only. This enters no new
 * creation path or body, and publishes no ready/executable Type. Failed output
 * is untouched; repeated exact environments reuse the scope.
 * [TPL-04](../../../../spec/language/templates.md#tpl-04) preserves owner/field equations.
 */
bool static_connection_scope(StaticConnectionPlan *, const StaticTermEnvironment *, const StaticTypedScope **);
/** Attach original checked incoming provenance separately from metadata scopes.
 * No default/root is invented. Repeated actual contexts allocate nothing.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves actual expansion obligations.
 */
bool static_connection_context(StaticConnectionPlan *, const StaticResultContext *, const StaticTypedScope **);
/** Select an already associated field by exact lexical binding identity; never
 * repair an earlier source use by looking up a later same-spelled name.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) fixes source bindings before Type solving.
 */
bool static_connection_field(StaticConnectionPlan *, const StaticTypedScope *, const StaticRuntimeField *, const StaticTypedField **);
/** Submit finite original ordinary annotation facts in a registered metadata
 * scope. No ordinary context/body is invented or certified; independently
 * concrete static exports retain their real supplier root. Changed counts
 * logical facts/requirements, never allocation snapshots. Failed output stays
 * untouched; partial work remains owned by its existing component.
 * [TPL-04](../../../../spec/language/templates.md#tpl-04) supplies field omission equations.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves actual expansion obligations.
 */
bool static_connection_annotations(StaticConnectionPlan *, const StaticTypedScope *, bool *);
/** Check original explicit annotations through a real incoming context and query
 * every required field Type, including inferred/local holes. Pending is retained
 * without suppressing later own-body equations. READY means closed field Types
 * and checked annotation paths, not ordinary body/capability admission. No caller
 * root is invented; failed outcomes stay unchanged and partial records owned.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) requires complete field Types.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) guards actual annotation dependencies.
 */
bool static_connection_validate_fields(StaticConnectionPlan *, const StaticResultContext *, StaticResultOutcome *);
/** Link every original foreach collection element Type to its exact lexical
 * binding under a real incoming context. Empty bodies still impose Type equality;
 * no runtime element is created and no ordinary body is certified. Repetition
 * reports progress only for new equations. Failure leaves output untouched.
 * [FLOW-05](../../../../spec/language/lists.md#flow-05) requires matching element/reference Types.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) preserves original local identities.
 */
bool static_connection_foreach(StaticConnectionPlan *, const StaticResultContext *, bool *);
/** Submit original declaration/default initializer Type equations through an
 * actual context. Every list initializer is one scalar element, never a splice.
 * Literals retain capability work and qualified constants retain independent
 * namespace work as Pending without inventing an anchor. Outputs are unchanged
 * on refusal; progress counts new equations only, never execution or admission.
 * [TYPE-02](../../../../spec/language/declarations.md#type-02) fixes scalar versus list initialization.
 * [TYPE-05](../../../../spec/language/declarations.md#type-05) requires exact element ownership form.
 */
bool static_connection_initializers(StaticConnectionPlan *, const StaticResultContext *, bool *, StaticResultOutcome *);
/** Submit local assignment Type equalities through a real incoming context.
 * Storage forms must match exactly. Qualified constants remain Pending for
 * their independent namespace service. Progress counts only new equations.
 * Outputs stay untouched on failure.
 * This operation does not certify capability/body admission or execute values.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) separates binding, equations and final checks.
 * [COPY-06](../../../../spec/language/values-and-references.md#copy-06) forbids direct continuation self-copy.
 */
bool static_connection_assignments(StaticConnectionPlan *, const StaticResultContext *, bool *, StaticResultOutcome *);
/** Obtain a source ordinary call's proven interface metadata under an actual
 * caller. Unknown target origins and constant reads remain Pending with operation
 * output untouched. Whole-list services use canonical intrinsic interfaces;
 * known scalar origins may retain symbolic captures;
 * this does not create/admit an actual callee or run its body. Selected scalar
 * operations resolve only through their parent's public result interface.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) forbids private/caller-name fallback.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) forbids origin guessing.
 */
bool static_connection_operation(StaticConnectionPlan *, const StaticResultContext *,
    const AstNode *, const StaticTypedOperation **, StaticResultOutcome *);
/** Generate an ordinary operation's input/result Type equations from its proven
 * interface. Unknown origins and independently deferred sources remain Pending;
 * known symbolic callee fields can be constrained by either direction. Neither
 * actual callee expansion nor capability/body admission follows from these facts.
 * Public outputs are unchanged on refusal; progress counts new equations only.
 * [TPL-03](../../../../spec/language/templates.md#tpl-03) derives specialization bindings from ordinary connections.
 * [CALL-08](../../../../spec/language/calls-and-results.md#call-08) preserves exact transfer forms.
 */
bool static_connection_bindings(StaticConnectionPlan *, const StaticResultContext *,
    const AstNode *, bool *, StaticResultOutcome *);
/** Resolve one retained qualified transfer using an explicitly supplied actual
 * namespace context. Selection must independently match the fixed namespace.
 * Add member/destination Type equality after exact storage checks; create no root
 * or runtime value. Changed reports a first namespace/storage proof or a new
 * equation; repeated proofs add no progress. Outputs remain untouched on refusal.
 * Capability, constant construction and ordinary body admission remain separate obligations.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) forbids receiver-driven namespace inference.
 */
bool static_connection_constant(StaticConnectionPlan *, const StaticTypedConstant *,
    const StaticResultContext *, bool *);
/** Register an original explicit shared-constant read and constrain its saved
 * output receivers after independent namespace closure. A null namespace context
 * only registers the obligation and reports Pending; it creates no default root.
 * A supplied context must exactly match the independently resolved namespace.
 * Outputs stay unchanged on refusal; no constant values or module bodies run.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) forbids receiver-driven namespace inference.
 */
bool static_connection_read(StaticConnectionPlan *, const StaticResultContext *,
    const AstNode *, const StaticResultContext *, bool *, StaticResultOutcome *);
/** Add corresponding field Type equalities only when present equations prove
 * owner expressions congruent. This can refine symbolic fields, but creates no
 * concrete specialization key or actual producer context. Changed reports new
 * equality facts, not allocations/snapshots. Failed output is untouched.
 * [TPL-04](../../../../spec/language/templates.md#tpl-04) requires per-owner field agreement.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) retains separate closed-key creation guards.
 */
bool static_connection_merge_fields(StaticConnectionPlan *, bool *);
#endif
