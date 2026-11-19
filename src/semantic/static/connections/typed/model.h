/** @file
 * @brief Associate fixed original runtime fields with source-owned Type terms in exact environments.
 * [TPL-04](../../../../../spec/language/templates.md#tpl-04) makes one concrete owner's field Type agree across uses.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) keeps lexical identities fixed during inference.
 */
#ifndef AMA_STATIC_TYPED_CONNECTION_MODEL_H
#define AMA_STATIC_TYPED_CONNECTION_MODEL_H
#include "../runtime/model.h"
#include "../../publication/types.h"

/** Field Types borrow only long-lived source/result/term identities. Inferred
 * and local holes have no fabricated static expression or formal symbol.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) permits unresolved original field Types.
 */
typedef struct StaticTypedField {
    struct StaticTypedField *next;
    const StaticRuntimeField *definition;
    const StaticTerm *type;
    StaticOrdinaryType *type_source;
} StaticTypedField;

/** A selected source has an element/scalar Type plus exact written storage form.
 * No runtime values or indices are evaluated while collecting Type equations.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) separates Type and storage inference.
 */
typedef struct {
    const StaticTerm *type;
    const StaticRuntimeField *field;
    bool reference;
    bool list;
} StaticTypedOperand;

/** An exact environment's field equations are metadata, never a concrete cache
 * admission or runtime instance. An owner expression may remain symbolic.
 * [TPL-04](../../../../../spec/language/templates.md#tpl-04) preserves per-owner field identity.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) still guards every actual definition expansion.
 */
typedef struct StaticTypedScope {
    struct StaticTypedScope *next;
    const StaticTermEnvironment *environment;
    const StaticTerm *owner;
    const StaticRuntimeModule *runtime;
    StaticTypedField *first_field;
    StaticTypedField *last_field;
    size_t field_count;
} StaticTypedScope;

/** Actual incoming paths are retained separately from source-only metadata.
 * Registering a nominal environment alone cannot enable body expansion.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves every required incoming ancestry.
 */
typedef struct StaticTypedContext {
    struct StaticTypedContext *next;
    const StaticResultContext *actual;
    const StaticTypedScope *scope;
} StaticTypedContext;
/** Source module interfaces and intrinsic list entries have distinct origins;
 * a service scope is not an executable module or a stored method value.
 * [LIST-04](../../../../../spec/language/lists.md#list-04) fixes these intrinsic interfaces.
 */
typedef enum { STATIC_OPERATION_SOURCE, STATIC_OPERATION_LIST_SERVICE,
    STATIC_OPERATION_LIST_METHOD } StaticTypedOperationKind;

/** Proven ordinary call interface metadata retains the caller and target
 * environments independently of actual callee expansion/admission. The target
 * may have unknown captures; original binding normalization remains reusable.
 * [CALL-08](../../../../../spec/language/calls-and-results.md#call-08) fixes original interface fields.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) separately requires actual callee creation paths.
 */
typedef struct StaticTypedOperation {
    struct StaticTypedOperation *next;
    const StaticTypedScope *caller;
    const AstNode *syntax;
    const StaticTypedScope *target;
    const BindingApplication *bindings;
    StaticTypedOperationKind kind;
    const StaticTerm *element;
    const StaticTerm *integer;
    bool reference_elements;
    SemanticListMethod method;
} StaticTypedOperation;
#endif
