/**
 * @file
 * @brief Classify supported statement shapes from physical structure without name lookup.
 *
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * Receiver assignment shape never depends on prior declarations.
 * [FLOW-01](../../../spec/language/control-flow.md#flow-01) distinguishes
 * owned conditions from operation refusal handlers by structure.
 */
#include "../internal.h"

/** Attach a parsed nonliteral source to a structural receiver and advance exactly past that source line; no first-use declaration inference occurs here.
 * Component contract: [COMP-01](../../../spec/implementation/compiler.md#comp-01), [FLOW-01](../../../spec/language/control-flow.md#flow-01). */
static bool assignment(Parser *parser, AstNode *node, AstAccess *receiver) {
    size_t indentation = parser->line->indentation + 1;
    parser->line = parser->line->next;
    parser_skip_region(parser, indentation);
    const LayoutLine *source = parser->line;
    if (source == NULL || source == parser->end || source->indentation != indentation
        || source->kind == LINE_COMMENT) {
        return parser_fail(parser, parser_site(parser), "An assignment source must stay in its receiver's child region", EXPECT_COPY_SOURCE);
    }
    node->kind = AST_ASSIGNMENT;
    node->as.assignment.receiver = receiver;
    if (!parser_source(parser, source, source->code, false, &node->as.assignment.source)) {
        return false;
    }
    node->span.end = source->code.end;
    parser->line = source->next;
    return true;
}

/** Output bindings and explicit modes start operation actions; an unmarked selection requires its own deeper action region.
 * A copy source has no child region, so this structural distinction never depends on a declared type or name.
 * Component contract: [COMP-01](../../../spec/implementation/compiler.md#comp-01), [FLOW-01](../../../spec/language/control-flow.md#flow-01). */
static bool operation_action(const LayoutLine *line, const LayoutLine *end) {
    const unsigned char *bytes = source_bytes(line->code.source);
    if (line->code.end > line->code.start) {
        unsigned char last = bytes[line->code.end - 1];
        if (last == '!' || last == '%') { return true; }
    }
    for (size_t offset = line->code.start; offset < line->code.end; offset++) {
        if (bytes[offset] == '<' || bytes[offset] == '>') { return true; }
    }
    const LayoutLine *marker = parser_condition_marker(line->next, end, line->indentation);
    if (marker != NULL && layout_text_is(marker->code, "{")) { return true; }
    bool blank = false;
    const LayoutLine *child = parser_significant(line->next, end, &blank);
    return child != NULL && child != end && child->indentation == line->indentation + 1;
}

/** Distinguish supported declaration, assignment, control or retained-read tails by physical structure only. Never query a declaration inventory to choose syntax.
 * Component contract: [COMP-01](../../../spec/implementation/compiler.md#comp-01), [FLOW-01](../../../spec/language/control-flow.md#flow-01). */
static bool bare_target(Parser *parser, ParseFrame **frame, AstNode *node) {
    const LayoutLine *line = parser->line;
    bool blank = false;
    const LayoutLine *next = parser_significant(line->next, parser->end, &blank);
    const LayoutLine *marker = parser_condition_marker(line->next, parser->end, line->indentation);
    if (marker != NULL && layout_text_is(marker->code, "{")
        && line->code.end >= line->code.start + 2
        && source_bytes(line->code.source)[line->code.end - 2] == '&') {
        return parser_foreach(parser, frame, node, marker);
    }
    AstAccess *access;
    if (!parser_access(parser, line->code, &access)) {
        return false;
    }
    bool present = next != NULL && next != parser->end;
    if (marker == NULL && present && next->indentation == line->indentation
        && (layout_text_is(next->code, "?") || layout_text_is(next->code, ":")
            || layout_text_is(next->code, "{"))) {
        return parser_fail(parser, line->code, "A control marker requires an adjacent condition with only same-depth comments",
            EXPECT_CONDITION_ADJACENCY);
    }
    if (marker != NULL && (layout_text_is(marker->code, "?")
            || layout_text_is(marker->code, ":") || layout_text_is(marker->code, "{"))) {
        return parser_control(parser, frame, node, access, marker);
    }
    if (present && next->indentation == line->indentation
        && next->code.start < next->code.end && source_bytes(next->code.source)[next->code.start] == '.') {
        return parser_declaration(parser, line->indentation, node);
    }
    if (present && next->indentation == line->indentation + 1) {
        return operation_action(next, parser->end)
            ? parser_start_call(parser, frame, node, line->code, AST_RETAINED_READ)
            : assignment(parser, node, access);
    }
    return parser_fail(parser, line->code, "A bare target is not an executable statement",
        EXPECT_STATEMENT_TAIL);
}

bool parser_statement(Parser *parser, ParseFrame **frame) {
    SourceSpan text = parser->line->code;
    AstNode *node = parser_node(parser, AST_CALL, text);
    if (node == NULL) {
        return false;
    }
    parser_append((*frame)->block, node);
    if (layout_text_is(text, "^") || layout_text_is(text, ">") || layout_text_is(text, "%")) {
        if (layout_text_is(text, "^")) {
            ParseFrame *loop = *frame;
            while (loop != NULL && loop->kind != FRAME_LOOP) { loop = loop->parent; }
            if (loop == NULL) {
                return parser_fail(parser, text, "Loop exit ^ requires an enclosing loop body", EXPECT_STATEMENT_TAIL);
            }
        }
        node->kind = layout_text_is(text, "^") ? AST_BREAK
            : layout_text_is(text, ">") ? AST_RETURN : AST_SUSPEND;
        parser->line = parser->line->next;
        return true;
    }
    const unsigned char *bytes = source_bytes(text.source);
    if (text.end > text.start && (bytes[text.end - 1] == '!' || bytes[text.end - 1] == '%')) {
        AstCallMode mode;
        SourceSpan target;
        return parser_call_mode(parser, text, &target, &mode)
            && parser_start_call(parser, frame, node, target, mode);
    }
    if (text.end > text.start && bytes[text.start] == '.') {
        return parser_namespace_statement(parser, frame, node);
    }
    return bare_target(parser, frame, node);
}
