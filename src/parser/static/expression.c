/**
 * @file
 * @brief Parse nested static expressions with explicit indentation and phase frames.
 *
 * [TPL-12](../../../spec/language/static-results.md#tpl-12) gives each input
 * expression its physical binding-line depth and orders inputs before outputs.
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01) excludes lookup
 * or recursive C activations from deciding this source structure.
 * [TPL-15](../../../spec/language/templates.md#tpl-15) adds a distinct
 * declaration mode with input-only children and a final named export selector.
 */
#include "internal.h"

/** One AST-arena frame owns an expression's input/output phase and child region.
 * The parent binding receives the child's final span on closure.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * makes dedentation return to the exact enclosing expression, not another name.
 */
typedef struct StaticExpressionFrame {
    struct StaticExpressionFrame *parent;
    AstTypeExpression *expression;
    AstStaticBinding *owner;
    size_t indentation;
    bool outputs_started;
    ParserTypeMode mode;
} StaticExpressionFrame;

/** Allocate a finite pending frame before following a nested input expression.
 * Component contract: [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * retains every partial frame within the AST's failure-cleanup owner.
 */
static StaticExpressionFrame *expression_frame(Parser *parser, StaticExpressionFrame *parent,
    AstTypeExpression *expression, AstStaticBinding *owner, size_t indentation, ParserTypeMode mode) {
    StaticExpressionFrame *frame = parser_allocate(parser, sizeof(*frame), expression->head);
    if (frame != NULL) {
        *frame = (StaticExpressionFrame){.parent = parent, .expression = expression,
            .owner = owner, .indentation = indentation, .mode = mode};
    }
    return frame;
}

/** Append one source-ordered binding, preserving first/last and monotone spans.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * retains named edge order and input-before-output producer phases.
 */
static void append_binding(AstTypeExpression *expression, AstStaticBinding *binding) {
    if (expression->last == NULL) { expression->first = binding; }
    else { expression->last->next = binding; }
    expression->last = binding;
    expression->span.end = binding->span.end;
}

/** Close one finite frame and extend its already-linked owning input binding.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * keeps nested source spans inside their exact owner without sibling consumption.
 */
static StaticExpressionFrame *close_expression(StaticExpressionFrame *frame) {
    if (frame->parent != NULL) {
        frame->owner->span.end = frame->expression->span.end;
        frame->parent->expression->span.end = frame->owner->span.end;
    }
    return frame->parent;
}

/** Advance physical lines or pop a finite pending frame on each transition.
 * Component contract: [TPL-12](../../../spec/language/static-results.md#tpl-12)
 * keeps nested bindings with their exact physical owner; a nonbinding line
 * closes frames and is checked by graph orchestration, never by Type lookup.
 */
bool parser_type_expression(Parser *parser, SourceSpan head, size_t indentation, ParserTypeMode mode,
    AstTypeExpression **output) {
    AstTypeExpression *expression;
    if (!static_expression_head(parser, head, &expression)) { return false; }
    StaticExpressionFrame *frame = expression_frame(parser, NULL, expression, NULL, indentation, mode);
    if (frame == NULL) { return false; }
    parser->line = parser->line->next;
    while (frame != NULL) {
        parser_skip_region(parser, frame->indentation + 1);
        const LayoutLine *line = parser->line;
        if (line == NULL || line == parser->end || line->indentation <= frame->indentation) {
            frame = close_expression(frame);
            continue;
        }
        if (line->indentation != frame->indentation + 1) {
            return parser_fail(parser, line->code, "A static binding cannot skip indentation levels", EXPECT_NESTING);
        }
        if (frame->mode == PARSER_TYPE_DECLARATION && parser_static_name(line->code)) {
            if (!static_expression_selector(parser, frame->expression)) { return false; }
            continue;
        }
        if (!static_binding_line(line)) {
            frame = close_expression(frame);
            continue;
        }
        if (frame->expression->selected_export.source != NULL) {
            return parser_fail(parser, line->code,
                "Declaration inputs must precede the export selector", EXPECT_STATIC_BINDING);
        }
        AstStaticBinding *binding;
        if (!static_binding_head(parser, &binding)) { return false; }
        if (binding->kind == AST_STATIC_OUTPUT && frame->mode != PARSER_TYPE_PRODUCER) {
            return parser_fail(parser, binding->marker,
                frame->mode == PARSER_TYPE_DECLARATION
                    ? "A declaration Type uses a bare export selector rather than an output binding"
                    : "A static value has named inputs only; select producer outputs in a graph statement",
                EXPECT_STATIC_BINDING);
        }
        if (binding->kind == AST_STATIC_INPUT && frame->outputs_started) {
            return parser_fail(parser, binding->marker, "Static inputs must precede outputs", EXPECT_STATIC_BINDING);
        }
        frame->outputs_started |= binding->kind == AST_STATIC_OUTPUT;
        append_binding(frame->expression, binding);
        parser->line = line->next;
        if (binding->input != NULL) {
            ParserTypeMode child_mode = frame->mode == PARSER_TYPE_DECLARATION
                ? PARSER_TYPE_DECLARATION : PARSER_TYPE_STATIC_VALUE;
            frame = expression_frame(parser, frame, binding->input, binding, line->indentation, child_mode);
            if (frame == NULL) { return false; }
        }
    }
    *output = expression;
    return true;
}
