/**
 * @file
 * @brief Represent separate ordinary and graph interface snapshots without runtime values.
 *
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) requires named
 * static inputs/outputs, separate from ordinary positional runtime interfaces.
 * [CALL-06](../../../../spec/language/calls-and-results.md#call-06) gives ordinary
 * inputs/results independent positions. [LIST-04](../../../../spec/language/lists.md#list-04)
 * provides canonical source-independent intrinsic field metadata.
 * [CALL-08](../../../../spec/language/calls-and-results.md#call-08) preserves label
 * storage forms. [TPL-09](../../../../spec/language/templates.md#tpl-09) selects
 * shared-constant surfaces independently of entry interfaces.
 */
#ifndef AMA_BINDING_SEQUENCE_MODEL_H
#define AMA_BINDING_SEQUENCE_MODEL_H
#include "../inventory.h"
#include "semantic/lists/interface.h"

/** These four roles are disjoint interfaces, not a shared advancing cursor.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
typedef enum { BINDING_STATIC_INPUT, BINDING_STATIC_OUTPUT,
    BINDING_RUNTIME_INPUT, BINDING_RUNTIME_OUTPUT, BINDING_SEQUENCE_COUNT } BindingDirection;

/** Complete borrowed name bytes can come from real source or immutable intrinsic
 * metadata. No synthetic SourceSpan is fabricated for an intrinsic field.
 * Contract: [LIST-04](../../../../spec/language/lists.md#list-04).
 */
typedef struct { const unsigned char *bytes; size_t length; SourceSpan site; } BindingName;

/** One immutable snapshot slot preserves actual original identity separately
 * from its filtered sequence position. Exactly one identity pointer is present.
 * Reference/list flags describe runtime label storage, not static argument kinds.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12),
 * [CALL-08](../../../../spec/language/calls-and-results.md#call-08).
 */
typedef struct BindingSlot {
    struct BindingSlot *next;
    BindingName name;
    size_t original_ordinal;
    size_t sequence_ordinal;
    const StaticSymbol *static_symbol;
    const AstNode *runtime_declaration;
    const SemanticListField *intrinsic_field;
    bool reference;
    bool list;
} BindingSlot;

/** Append during construction only; all links/counts are immutable after the
 * containing interface is successfully published.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
typedef struct { BindingSlot *first; BindingSlot *last; size_t count; } BindingSequence;

/** Shared reads expose constants instead of runtime call inputs/results.
 * Contract: [TPL-09](../../../../spec/language/templates.md#tpl-09).
 */
typedef enum { BINDING_MODULE_SURFACE, BINDING_CONSTANT_SURFACE } BindingRuntimeSurface;

/** Distinct source and intrinsic origins never acquire identity from matching
 * field layouts. Intrinsic identity includes its canonical method enumeration.
 * Contract: [LIST-04](../../../../spec/language/lists.md#list-04).
 */
typedef enum { BINDING_SOURCE_INTERFACE, BINDING_LIST_INTERFACE } BindingInterfaceKind;

/** A published interface owns snapshots and borrows its actual source/static or
 * canonical intrinsic origin. This record pointer is not a type identity.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
typedef struct BindingInterface {
    struct BindingInterface *next;
    BindingInterfaceKind kind;
    const StaticModule *origin;
    SemanticListMethod list_method;
    BindingRuntimeSurface surface;
    BindingSequence sequences[BINDING_SEQUENCE_COUNT];
} BindingInterface;
/** Empty means no request selected a mode; nested operations choose separately.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
typedef enum { BINDING_MODE_EMPTY, BINDING_MODE_NAMED, BINDING_MODE_POSITIONAL } BindingMode;

/** A static expression and an ordinary operation have independent direction
 * counters, even when a namespace expression is attached to a runtime read.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
typedef enum { BINDING_STATIC_APPLICATION, BINDING_RUNTIME_APPLICATION } BindingApplicationKind;

/** Preserve one exact source request and its actual snapshot slot. A null slot
 * denotes unresolved actual origin, never an expected-signature position. The
 * original operands remain borrowed syntax, not evaluated or copied values.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12),
 * [CALL-06](../../../../spec/language/calls-and-results.md#call-06).
 */
typedef struct BindingRequest {
    struct BindingRequest *next;
    SourceSpan span;
    SourceSpan marker;
    SourceSpan label;
    BindingDirection direction;
    const BindingSlot *slot;
    size_t source_ordinal;
    bool named;
    bool omitted;
    bool reference;
    bool list;
    union {
        const AstStaticBinding *static_binding;
        const AstArgument *runtime_binding;
    } syntax;
} BindingRequest;

/** One immutable normalized operation. Pending records keep raw requests until
 * the actual origin is independently supplied. A resolved record certifies only
 * field identities/sequence checks, not type equations or producer closure.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
typedef struct BindingApplication {
    struct BindingApplication *next;
    BindingApplicationKind kind;
    BindingMode mode;
    const BindingInterface *interface;
    const AstTypeExpression *expression;
    const AstNode *operation;
    BindingRequest *first;
    BindingRequest *last;
    size_t input_count;
    size_t output_count;
    bool pending;
} BindingApplication;
#endif
