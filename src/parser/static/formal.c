/**
 * @file
 * @brief Parse named constructor signatures with explicit source-depth frames.
 *
 * [TPL-07](../../../spec/language/templates.md#tpl-07) requires a nonempty
 * signature when children are present and keeps its foreign names structural.
 * [SYN-02](../../../spec/language/source-and-names.md#syn-02) forbids comments
 * from introducing a deeper region or reopening it after dedentation.
 */
#include "internal.h"

/** An AST-owned pending signature has one fixed source indentation and parent.
 * Component contract: [TPL-07](../../../spec/language/templates.md#tpl-07)
 * permits finite recursively nested signatures without a parser depth budget.
 */
typedef struct FormalFrame {
    struct FormalFrame *parent;
    AstStaticFormal *formal;
    size_t indentation;
} FormalFrame;

/** Validate one exact header before allocating its source-preserving record.
 * Component contract: [TPL-07](../../../spec/language/templates.md#tpl-07)
 * rejects defaults, hanging markers and storage suffixes in static headers.
 */
static AstStaticFormal *formal_head(Parser *parser) {
    SourceSpan name = parser->line->code;
    if (!parser_static_name(name)) {
        (void)parser_fail(parser, name, "A static formal needs one complete static name", EXPECT_STATIC_FORMAL);
        return NULL;
    }
    AstStaticFormal *formal = parser_allocate(parser, sizeof(*formal), name);
    if (formal != NULL) { formal->name = name; formal->span = name; }
    return formal;
}

/** Prepare a child traversal frame within the AST failure-cleanup owner.
 * Component contract: [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * retains finite source nesting explicitly instead of recursive C calls.
 */
static FormalFrame *formal_frame(Parser *parser, FormalFrame *parent,
    AstStaticFormal *formal, size_t indentation) {
    FormalFrame *frame = parser_allocate(parser, sizeof(*frame), formal->name);
    if (frame != NULL) { *frame = (FormalFrame){parent, formal, indentation}; }
    return frame;
}

/** A comment-only deeper region cannot supply a nonempty signature.
 * Component contract: [SYN-02](../../../spec/language/source-and-names.md#syn-02)
 * keeps comments in existing regions; [TPL-07](../../../spec/language/templates.md#tpl-07)
 * requires a real signature member, whereas blank lines may attach to the parent.
 */
static bool empty_signature_layout(Parser *parser, const LayoutLine *first) {
    for (const LayoutLine *line = first; line != parser->line; line = line->next) {
        if (line->kind == LINE_COMMENT) {
            return parser_fail(parser, line->code,
                "A deeper signature comment requires a signature member", EXPECT_STATIC_FORMAL);
        }
    }
    return true;
}

/** Consume one header per push or close one finite frame per nonconsuming step.
 * Component contract: [TPL-07](../../../spec/language/templates.md#tpl-07)
 * retains complete named signatures; no surrounding formal table is consulted.
 */
bool parser_static_formal(Parser *parser, size_t indentation, AstStaticFormal **output) {
    AstStaticFormal *root = formal_head(parser);
    if (root == NULL) { return false; }
    FormalFrame *frame = formal_frame(parser, NULL, root, indentation);
    if (frame == NULL) { return false; }
    parser->line = parser->line->next;
    while (frame != NULL) {
        const LayoutLine *before_layout = parser->line;
        parser_skip_region(parser, frame->indentation + 1);
        const LayoutLine *line = parser->line;
        if (line == NULL || line == parser->end || line->indentation <= frame->indentation) {
            if (frame->formal->first == NULL && !empty_signature_layout(parser, before_layout)) { return false; }
            if (frame->parent != NULL) { frame->parent->formal->span.end = frame->formal->span.end; }
            frame = frame->parent;
            continue;
        }
        if (line->indentation != frame->indentation + 1) {
            return parser_fail(parser, line->code, "A signature cannot skip indentation levels", EXPECT_NESTING);
        }
        AstStaticFormal *child = formal_head(parser);
        if (child == NULL) { return false; }
        if (frame->formal->last == NULL) { frame->formal->first = child; }
        else { frame->formal->last->next = child; }
        frame->formal->last = child;
        FormalFrame *next = formal_frame(parser, frame, child, line->indentation);
        if (next == NULL) { return false; }
        frame = next;
        parser->line = line->next;
    }
    *output = root;
    return true;
}
