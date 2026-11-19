/** @file
 * @brief Retain source-ordered runtime constructs independently of Type inference.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) fixes lexical occurrence order.
 */
#ifndef AMA_STATIC_RUNTIME_EVENTS_MODEL_H
#define AMA_STATIC_RUNTIME_EVENTS_MODEL_H
#include "../../../model.h"

/** Actual operand positions, not AST storage grouping, determine event order.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) visits interleaved calls lexically.
 */
typedef enum { STATIC_RUNTIME_DECLARATION, STATIC_RUNTIME_INITIALIZER,
    STATIC_RUNTIME_ASSIGNMENT, STATIC_RUNTIME_CALL, STATIC_RUNTIME_BINDING,
    STATIC_RUNTIME_CONDITION, STATIC_RUNTIME_FOREACH } StaticRuntimeEventKind;

/** Immutable original syntax/section metadata; source/argument are populated
 * only by their respective event kinds. No runtime field or Type is resolved.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) separates syntax and inference.
 */
typedef struct StaticRuntimeEvent {
    struct StaticRuntimeEvent *next;
    StaticRuntimeEventKind kind;
    SourceSpan site;
    const AstNode *node;
    StaticContext context;
    const AstSource *source;
    const AstArgument *argument;
} StaticRuntimeEvent;
#endif
