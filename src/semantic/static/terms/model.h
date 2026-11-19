/**
 * @file
 * @brief Represent scoped variables and normalized nominal values without runtime instance state.
 *
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) preserves original
 * origins and every capture. [TPL-08](../../../../spec/language/templates.md#tpl-08)
 * separates local residual binders from surrounding free/inference variables.
 * [TPL-10](../../../../spec/language/templates.md#tpl-10) keeps source and
 * term structure distinct from equation solutions and executable layout.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) keys runtime
 * receiver/foreach holes by original source and environment identities.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) gives static
 * producers compiler-only input-map identities without making them Type values.
 */
#ifndef AMA_STATIC_TERM_MODEL_H
#define AMA_STATIC_TERM_MODEL_H
#include "../model.h"

/** Forward owner identity; none of these records is independently freed.
 * Contract: [TPL-10](../../../../spec/language/templates.md#tpl-10).
 */
typedef struct StaticTermStore StaticTermStore;
/** Forward immutable value identity.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
typedef struct StaticTerm StaticTerm;
/** Forward lexical/normalized owner environment.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
typedef struct StaticTermEnvironment StaticTermEnvironment;

/** A residual is a locally bound original constructor formal, never a solution
 * hole. Nested Template signatures remain available through the original symbol.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
typedef struct StaticTermResidual {
    struct StaticTermResidual *next;
    const StaticSymbol *formal;
} StaticTermResidual;

/** Fixed maps retain even unused values in original declaration order.
 * Contract: [TPL-05](../../../../spec/language/templates.md#tpl-05).
 */
typedef struct StaticTermCapture {
    struct StaticTermCapture *next;
    const StaticSymbol *formal;
    const StaticTerm *value;
} StaticTermCapture;

/** Nominal metadata never unfolds fields or source bodies. The captures and
 * residuals partition the original origin's complete formal set.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
typedef struct {
    const StaticModule *origin;
    StaticTermCapture *first_capture;
    StaticTermCapture *last_capture;
    size_t capture_count;
    StaticTermResidual *first_residual;
    StaticTermResidual *last_residual;
    size_t residual_count;
} StaticNominalTerm;

/** A variable's identity is distinct from its later solver solution. Derived
 * output names may have deferred category; no role silently becomes Type.
 * Contract: [TPL-10](../../../../spec/language/templates.md#tpl-10).
 */
typedef enum { STATIC_VARIABLE_FORMAL, STATIC_VARIABLE_DERIVED,
    STATIC_VARIABLE_FIELD, STATIC_VARIABLE_PRODUCER, STATIC_VARIABLE_APPLICATION, STATIC_VARIABLE_RUNTIME } StaticVariableRole;

/** Inference keys retain both lexical occurrence and original residual formal.
 * A symbol reference instead uses the owner's exact local symbol identity.
 * Application results have no symbol; producer points to their original expression.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
typedef struct {
    StaticVariableRole role;
    const StaticTermEnvironment *environment;
    const StaticSymbol *symbol;
    const AstNode *declaration;
    const AstTypeExpression *producer;
    /** Original simple runtime receiver; null for a foreach-header Type hole.
     * [TYPE-06](../../../../spec/language/declarations.md#type-06) preserves lexical field identity.
     */
    const AstAccess *runtime_access;
} StaticVariableIdentity;

/** Value graphs contain normalized nominal records and scoped variables only;
 * pending source applications are represented separately, not guessed values.
 * Contract: [TPL-10](../../../../spec/language/templates.md#tpl-10).
 */
typedef enum { STATIC_TERM_VARIABLE, STATIC_TERM_NOMINAL } StaticTermKind;

/** Published immutable graph vertex. Signatures are known/deferred metadata,
 * not a wildcard equality rule. Source provenance does not define value identity.
 * Contract: [TPL-05](../../../../spec/language/templates.md#tpl-05).
 */
struct StaticTerm {
    StaticTerm *next;
    const StaticTermStore *owner;
    size_t ordinal;
    StaticTermKind kind;
    StaticCategory category;
    const StaticSignature *signature;
    SourceSpan site;
    bool ground;
    union { StaticVariableIdentity variable; StaticNominalTerm nominal; } as;
};

/** Generic definitions have no applied key. Ordinary applied environments borrow
 * a fully bound nominal Type; static producers retain a separate complete input
 * map. Both may contain symbolic values until the solver closes them.
 * Contract: [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
struct StaticTermEnvironment {
    StaticTermEnvironment *next;
    const StaticModule *definition;
    const StaticTerm *owner_type;
    /** Complete original input map for an applied static producer; these compiler
     * keys are not Type/Template values. Null also represents a zero-input map;
     * producer_applied distinguishes it from a generic scope.
     * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
     */
    const StaticTermCapture *producer_inputs;
    bool producer_applied;
};
#endif
