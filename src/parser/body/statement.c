/**
 * @file
 * @brief Classify supported statement shapes from physical structure without name lookup.
 *
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * The retained adjacent-marker/compact-call subset is not full current control or spacing conformance.
 */
#include "../internal.h"

/** Attach a parsed nonliteral source to a structural receiver and advance exactly past that source line; no first-use declaration inference occurs here.
 */
static bool assignment(Parser *parser, AstNode *node, AstAccess *receiver, const LayoutLine *source) {
    node->kind = AST_ASSIGNMENT;
    node->as.assignment.receiver = receiver;
    if (!parser_source(parser, source, source->code, false, &node->as.assignment.source)) {
        return false;
    }
    node->span.end = source->code.end;
    parser->line = source->next;
    return true;
}

/** Attach an adjacent owned-syntax condition to a retained branch/loop frame. Current blank-adjacency restriction is an implementation gap assigned to T027.
 */
static bool control(Parser *parser, ParseFrame **frame, AstNode *node,
    AstAccess *condition, const LayoutLine *marker, bool blank) {
    if (blank || !parser_owned_access(condition)) {
        return parser_fail(parser, node->span, blank ? "A blank line separates the condition from its marker"
                : "A condition requires owned-value syntax",
            EXPECT_CONDITION_ADJACENCY);
    }
    bool loop = layout_text_is(marker->code, "{");
    node->kind = loop ? AST_LOOP : AST_BRANCH;
    node->as.control.condition = condition;
    node->span.end = marker->code.end;
    parser->line = marker->next;
    return parser_push_frame(parser, frame, &node->as.control.body, node,
        (*frame)->indentation + 1, loop ? FRAME_LOOP : FRAME_BRANCH);
}

/** Distinguish supported declaration, assignment, control or retained-read tails by physical structure only. Never query a declaration inventory to choose syntax.
 */
static bool bare_target(Parser *parser, ParseFrame **frame, AstNode *node) {
    const LayoutLine *line = parser->line;
    bool blank = false;
    const LayoutLine *next = parser_significant(line->next, parser->end, &blank);
    AstAccess *access;
    if (!parser_access(parser, line->code, &access)) {
        return false;
    }
    bool present = next != NULL && next != parser->end;
    if (present && next->indentation == line->indentation
        && (layout_text_is(next->code, "?") || layout_text_is(next->code, "{"))) {
        return control(parser, frame, node, access, next, blank);
    }
    if (present && next->indentation == line->indentation
        && next->code.start < next->code.end && source_bytes(next->code.source)[next->code.start] == '.') {
        return parser_declaration(parser, line->indentation, node);
    }
    if (present && next->indentation == line->indentation + 1) {
        const unsigned char *bytes = source_bytes(next->code.source);
        bool binding = false;
        for (size_t offset = next->code.start; offset < next->code.end; offset++) {
            binding |= bytes[offset] == '<' || bytes[offset] == '>';
        }
        return binding ? parser_start_call(parser, frame, node, line->code, AST_RETAINED_READ)
            : assignment(parser, node, access, next);
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
        node->kind = layout_text_is(text, "^") ? AST_BREAK
            : layout_text_is(text, ">") ? AST_RETURN : AST_SUSPEND;
        parser->line = parser->line->next;
        return true;
    }
    const unsigned char *bytes = source_bytes(text.source);
    if (text.end > text.start && (bytes[text.end - 1] == '!' || bytes[text.end - 1] == '%')) {
        AstCallMode mode = bytes[text.end - 1] == '!' ? AST_ENTRY_CALL : AST_RESUME_CALL;
        SourceSpan target = text;
        target.end--;
        return parser_start_call(parser, frame, node, target, mode);
    }
    if (text.end > text.start && bytes[text.start] == '.') {
        return parser_start_call(parser, frame, node, text, AST_CONSTANT_READ);
    }
    return bare_target(parser, frame, node);
}
