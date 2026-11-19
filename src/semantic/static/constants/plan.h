/** @file
 * @brief Own dependent shared declaration and namespace metadata without constructing constant values.
 * [CONST-06](../../../../spec/language/declarations.md#const-06) substitutes exact owner parameters.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) requires concrete namespace selection.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates metadata lifetime.
 */
#ifndef AMA_STATIC_CONSTANT_PLAN_H
#define AMA_STATIC_CONSTANT_PLAN_H
#include "../publication/types.h"

/** One original shared declaration under a checked incoming scope. Its Type
 * variable is source/environment scoped even when different paths retain uses.
 * List storage is distinct from its element Type; no runtime value is stored.
 * [CONST-06](../../../../spec/language/declarations.md#const-06) preserves dependent declarations.
 */
typedef struct StaticConstantDeclaration {
    struct StaticConstantDeclaration *next;
    const StaticResultContext *context;
    const AstNode *syntax;
    StaticOrdinaryType *source;
    const StaticTerm *type;
    bool list;
} StaticConstantDeclaration;

/** Deferred saved reads retain raw requests without using requested members
 * or receivers to determine their namespace. Selection is a later operation.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) requires independent namespace closure.
 */
typedef struct StaticConstantRead {
    struct StaticConstantRead *next;
    const StaticResultContext *context;
    const AstNode *syntax;
    StaticOrdinaryType *source;
    const BindingApplication *requests;
} StaticConstantRead;

/** A selected own shared declaration supplies a scoped Type obligation for
 * later typed connections; its runtime value is neither read nor constructed.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) selects own constants only.
 */
typedef struct StaticConstantMember {
    struct StaticConstantMember *next;
    const BindingRequest *request;
    const StaticConstantDeclaration *declaration;
} StaticConstantMember;

/** Member identities are exposed only after independently closing the namespace.
 * Declared member Types may still require the ordinary typed-connection solver.
 * [CONST-06](../../../../spec/language/declarations.md#const-06) separates dependent types and construction.
 */
typedef struct StaticConstantSelection {
    struct StaticConstantSelection *next;
    const StaticConstantRead *read;
    const StaticResultContext *context;
    const StaticTerm *namespace_type;
    const BindingApplication *requests;
    StaticConstantMember *first;
    size_t count;
} StaticConstantSelection;

/** Original qualified source registration retains the loaded namespace identity
 * without inventing application syntax or selecting an open namespace's member.
 * [CONST-02](../../../../spec/language/declarations.md#const-02) preserves qualified value sources.
 */
typedef struct StaticConstantSource {
    struct StaticConstantSource *next;
    const StaticResultContext *context;
    const StaticConstantOccurrence *occurrence;
    const StaticTerm *namespace_type;
} StaticConstantSource;

/** A qualified source selects its own shared declaration in an independently
 * checked fixed namespace. Type obligations remain separate from runtime values.
 * [CONST-02](../../../../spec/language/declarations.md#const-02) checks storage/member identity.
 */
typedef struct StaticConstantValue {
    struct StaticConstantValue *next;
    const StaticConstantSource *source;
    const StaticResultContext *context;
    const StaticConstantDeclaration *declaration;
} StaticConstantValue;

/** One original shared initializer retains typed source evidence without
 * constructing a value. Exactly one local/qualified source is set, or both are
 * null for a literal; literals never provide an intrinsic Type.
 * [CONST-06](../../../../spec/language/declarations.md#const-06) permits typed initializer equalities.
 */
typedef struct StaticConstantInitializer {
    struct StaticConstantInitializer *next;
    const StaticConstantDeclaration *declaration;
    const AstSource *syntax;
    const StaticConstantDeclaration *local;
    const StaticConstantSource *qualified;
} StaticConstantInitializer;

/** Unresolved final namespaces differ from invalid identities, conflicting
 * storage/types and resource exhaustion. Failed owners are destructible only.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) rejects open namespaces.
 */
typedef enum { STATIC_CONSTANT_OK, STATIC_CONSTANT_INVALID, STATIC_CONSTANT_CONFLICT,
    STATIC_CONSTANT_UNRESOLVED, STATIC_CONSTANT_OUT_OF_MEMORY } StaticConstantStatus;

/** Destroy before publications and all borrowed static/source owners. This
 * owner never initializes constants or creates native/runtime instances.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates owners.
 */
typedef struct {
    SourceArena arena;
    StaticPublicationPlan *publications;
    StaticConstantDeclaration *first_declaration;
    size_t declaration_count;
    StaticConstantRead *first_read;
    size_t read_count;
    StaticConstantSelection *first_selection;
    size_t selection_count;
    StaticConstantSource *first_source;
    size_t source_count;
    StaticConstantValue *first_value;
    size_t value_count;
    StaticConstantInitializer *first_initializer;
    size_t initializer_count;
    StaticConstantStatus status;
    Diagnostic error;
} StaticConstantPlan;

/** Borrow healthy publication ownership without allocation or semantic execution.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates lifetimes.
 */
void static_constant_init(StaticConstantPlan *, StaticPublicationPlan *, SourceAllocator);
/** Release only owned partial metadata; borrowed source/value records survive.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds teardown.
 */
void static_constant_destroy(StaticConstantPlan *);
/** Bind a registered original top-level shared declaration in a checked scope.
 * Its element Type has independent declaration metadata and a closure obligation;
 * no literal/default/body evaluation or new formal occurs. Publish only on success.
 * [CONST-06](../../../../spec/language/declarations.md#const-06) binds declared owner types.
 * [CONST-01](../../../../spec/language/declarations.md#const-01) forbids reference storage.
 */
bool static_constant_declaration(StaticConstantPlan *, const StaticResultContext *,
    const AstNode *, const StaticConstantDeclaration **);
/** Register one original explicit constant read with unresolved own requests.
 * No member lookup, namespace inference or runtime value copy occurs. Repeated
 * registration reuses the exact source/context identity; failed output is untouched.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) separates registration from selection.
 */
bool static_constant_read(StaticConstantPlan *, const StaticResultContext *,
    const AstNode *, const StaticConstantRead **);
/** Independently close the read namespace, then select own declarations using
 * actual original names/positions. No requested member or receiver anchors it.
 * Optional context must be a checked actual namespace scope matching that Type;
 * null may use an actual normal-source Type context. Static export selection and
 * plain Type reuse need a checked ordinary scope
 * supplied by the required-type registry rather than an invented default root.
 * Selected declaration Type terms remain available for later constraints.
 * Failed output is untouched; no runtime constant value is copied.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) requires concrete namespaces.
 */
bool static_constant_select(StaticConstantPlan *, const StaticConstantRead *,
    const StaticResultContext *, const StaticConstantSelection **);
/** Register an exact inventoried qualified source in its actual caller scope.
 * No static syntax is fabricated; requested member/receiver cannot infer Type.
 * Publish only on success; partial records remain owned on failure.
 * [CONST-02](../../../../spec/language/declarations.md#const-02) fixes the written namespace.
 */
bool static_constant_source(StaticConstantPlan *, const StaticResultContext *,
    const AstSource *, const StaticConstantSource **);
/** Select only after fixed namespace closure and exact explicit checked scope.
 * Qualified syntax cannot supply missing template arguments. Shared storage and
 * own member existence are checked; values are never initialized or executed.
 * Failed output is untouched, repeated exact selections reuse their descriptor.
 * [CONST-02](../../../../spec/language/declarations.md#const-02) preserves qualified semantics.
 * [CONST-05](../../../../spec/language/declarations.md#const-05) requires a concrete namespace.
 */
bool static_constant_value(StaticConstantPlan *, const StaticConstantSource *,
    const StaticResultContext *, const StaticConstantValue **);
/** Register an original shared initializer and its permitted typed source.
 * Literals add no Type evidence. Local sources must name own shared declarations;
 * value initialization order and ordinary storage/index checks remain later work.
 * Failures leave output untouched; partial records retain one arena owner.
 * [CONST-06](../../../../spec/language/declarations.md#const-06) preserves initializer obligations.
 */
bool static_constant_initializer(StaticConstantPlan *, const StaticConstantDeclaration *,
    const AstSource *, const StaticConstantInitializer **);
/** Evaluate static Type expressions and equate typed source/destination terms.
 * Qualified sources require an explicit independently checked namespace context;
 * other sources ignore that argument. READY certifies checked closed Type terms,
 * never native capability, value construction, ordering or executable admission.
 * Pending retains actual equations; failure leaves outcome untouched.
 * [CONST-06](../../../../spec/language/declarations.md#const-06) separates Type inference from values.
 */
bool static_constant_initializer_resolve(StaticConstantPlan *, const StaticConstantInitializer *,
    const StaticResultContext *, StaticResultOutcome *);
#endif
