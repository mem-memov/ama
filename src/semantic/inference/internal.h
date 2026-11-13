/**
 * @file
 * @brief Declare one source-ordered inference domain with exact lexical bindings and typed operation interfaces.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) classifies first
 * receivers before typing later constructs; [NAME-06](../../../spec/language/declarations.md#name-06)
 * gives foreach locals lexical intervals without module-name occurrences.
 */
#ifndef AMA_SEMANTIC_INFERENCE_INTERNAL_H
#define AMA_SEMANTIC_INFERENCE_INTERNAL_H
#include "semantic/internal.h"
/** Source constructs are ordered at their actual written locations, including interleaved call bindings. */
typedef enum { INFER_DECLARATION, INFER_INITIALIZER, INFER_ASSIGNMENT, INFER_CALL,
    INFER_BINDING, INFER_CONDITION, INFER_FOREACH } InferenceEventKind;
/** Arena-owned event borrowing syntax, optional binding/source and initializing declaration. */
typedef struct InferenceEvent {
    struct InferenceEvent *next;
    SourceSpan site;
    InferenceEventKind kind;
    const AstNode *node;
    const AstArgument *argument;
    const AstSource *source;
    SemanticSymbol *initializing;
} InferenceEvent;
/** A module name encountered at an actual source occurrence, independent of declaration availability. */
typedef struct EncounteredName { struct EncounteredName *next; SourceSpan name; } EncounteredName;
/** Exact transfer/selection shape; type identity is never inferred from runtime contents. */
typedef struct { SemanticType *type; SemanticSymbol *symbol; bool reference; bool list; } InferenceValue;
/** Compiler-provided list interface metadata is distinct from ordinary module-valued results. */
typedef enum { INFER_INSTANCE, INFER_NAMESPACE, INFER_LIST_SERVICE, INFER_LIST_METHOD } InferenceOperationKind;
/** The fixed service entry names select typed interfaces, never action constants. */
typedef enum { INFER_APPEND, INFER_INSERT, INFER_REMOVE, INFER_CLEAR, INFER_SWAP, INFER_LENGTH } InferenceListMethod;
/** Receiver classification precedes its adjacent source; fresh fields have no fabricated declaration syntax. */
typedef struct { const AstAccess *syntax; InferenceValue value; bool fresh; } InferenceReceiver;
/** One typed operation prefix, cached by exact AST identity after its written target is reached. */
typedef struct InferenceOperation {
    struct InferenceOperation *next;
    const AstNode *syntax;
    InferenceOperationKind kind;
    InferenceListMethod method;
    InferenceValue value;
} InferenceOperation;
/** Semantic-owned event/encounter/operation work, borrowing its program and module type. */
typedef struct {
    SemanticProgram *program;
    SemanticType *type;
    InferenceEvent *first;
    InferenceEvent *last;
    size_t event_count;
    EncounteredName *encountered;
    InferenceOperation *operations;
    SourceSpan inferring;
} Inference;
/** Collect explicit source constructs without revisiting alias links; ordinary operands are not processed yet. */
bool inference_collect(Inference *);
/** Sort event links by original source position, preserving same-site collection order. */
void inference_sort_events(Inference *);
/** Record an actual module occurrence and optionally return whether it had already occurred. */
bool inference_encounter(Inference *, SourceSpan, bool *);
/** Encounter one caller name and resolve lexical/module storage at its exact source site. */
SemanticSymbol *inference_name(Inference *, SourceSpan, SourceSpan, bool);
/** Resolve one access and visit its names in order, respecting lexical locals and construction restrictions. */
bool inference_access(Inference *, const AstAccess *, bool, InferenceValue *);
/** Resolve a raw assignment/input/initializer source and its named occurrences without copy-capability guessing. */
bool inference_source(Inference *, const AstSource *, bool, InferenceValue *);
/** Process an assignment receiver/source as one adjacent lexical construct. */
bool inference_assignment(Inference *, const AstNode *);
/** Process one ordinary input/output binding at its written source location. */
bool inference_binding(Inference *, const InferenceEvent *);
/** Type a root/selected operation only through its declared interface and caller operands. */
bool inference_operation(Inference *, const AstNode *);
/** Retrieve a prefix already reached in source order; missing returns null without body/name fallback. */
InferenceOperation *inference_find_operation(const Inference *, const AstNode *);
/** Select one named result access and caller-side indices from an already typed operation. */
bool inference_select_result(Inference *, const InferenceOperation *, const AstAccess *, InferenceValue *);
/** Select one declared output, including the fixed typed list-method result interface. */
bool inference_result(Inference *, const InferenceOperation *, const AstAccess *, size_t, SourceSpan, InferenceValue *);
/** Encounter/classify a receiver before visiting its assignment source; retain only exact typed existing storage. */
bool inference_begin_receiver(Inference *, const AstAccess *, InferenceReceiver *);
/** Finish a classified receiver with the exact source/result shape and append fresh persistent storage. */
bool inference_finish_receiver(Inference *, const InferenceReceiver *, const InferenceValue *);
/** Classify/type one simple or indexed receiver against an exact result/source transfer shape. */
bool inference_receiver(Inference *, const AstAccess *, const InferenceValue *);
/** Resolve a foreach collection and introduce/reuse its exact lexical scalar-reference cell. */
bool inference_foreach(Inference *, const AstNode *);
/** Merge explicit/inferred fields in declaration order and recompute normal role/module ordinals. */
void inference_order_fields(SemanticType *);
#endif
