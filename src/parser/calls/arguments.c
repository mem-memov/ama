#include "../internal.h"

static bool parse_binding(Parser *parser, AstArgument *argument) {
    SourceSpan text = parser->line->code;
    const unsigned char *bytes = source_bytes(text.source);
    size_t marker = text.start;
    while (marker < text.end && bytes[marker] != '<' && bytes[marker] != '>') {
        marker++;
    }
    if (marker == text.end) {
        return parser_fail(parser, text, "Expected a call binding", "< source, > receiver, or >> receiver");
    }
    if (marker != text.start) {
        if (bytes[marker - 1] != ' ') {
            return parser_fail(parser, text, "Named binding components need spaces", "name < source, name > receiver, or name >> receiver");
        }
        SourceSpan label = {text.source, text.start, marker};
        while (label.end > label.start && bytes[label.end - 1] == ' ') {
            label.end--;
        }
        if (!parser_access(parser, label, &argument->label)
            || !parser_declaration_name(argument->label)) {
            return parser_fail(parser, label, "A binding label must be a declaration name", "name, name&, name[] or name&[]");
        }
    }
    size_t end = marker + 1;
    if (bytes[marker] == '>' && end < text.end && bytes[end] == '>') {
        end++;
    }
    argument->kind = bytes[marker] == '<' ? AST_INPUT
        : end - marker == 2 ? AST_SAVED_OUTPUT : AST_OUTPUT;
    argument->marker = (SourceSpan){text.source, marker, end};
    if (end == text.end || bytes[end] != ' ') {
        return parser_fail(parser, text, "A binding requires a separated operand", "a space and source/receiver after the marker");
    }
    while (end < text.end && bytes[end] == ' ') {
        end++;
    }
    SourceSpan operand = {text.source, end, text.end};
    return argument->kind == AST_INPUT
        ? parser_source(parser, parser->line, operand, false, &argument->source)
        : parser_access(parser, operand, &argument->receiver);
}

static bool append_argument(Parser *parser, AstNode *node, AstArgument *argument) {
    bool named = argument->label != NULL;
    if (node->as.call.first != NULL && named != node->as.call.named) {
        return parser_fail(parser, argument->span, "Named and positional bindings cannot mix", "one binding style throughout a call");
    }
    AstCallMode mode = node->as.call.mode;
    if ((mode == AST_ENTRY_CALL && argument->kind == AST_SAVED_OUTPUT)
        || (mode == AST_RESUME_CALL && argument->kind != AST_OUTPUT)
        || ((mode == AST_RETAINED_READ || mode == AST_CONSTANT_READ) && argument->kind != AST_SAVED_OUTPUT)
        || (argument->kind == AST_INPUT && node->as.call.output_count != 0)) {
        return parser_fail(parser, argument->marker, "Binding marker is not allowed in this call mode/order",
            mode == AST_ENTRY_CALL ? "inputs < before outputs >" : mode == AST_RESUME_CALL ? "outputs > only" : "retained outputs >> only");
    }
    node->as.call.named = named;
    argument->position = argument->kind == AST_INPUT
        ? node->as.call.input_count++ : node->as.call.output_count++;
    if (node->as.call.last == NULL) {
        node->as.call.first = argument;
    } else {
        node->as.call.last->next = argument;
    }
    node->as.call.last = argument;
    node->span.end = argument->span.end;
    return true;
}

bool parser_call_arguments(Parser *parser, AstNode *node, size_t indentation) {
    parser_skip_layout(parser);
    while (parser->line != NULL && parser->line != parser->end
        && parser->line->indentation == indentation) {
        AstArgument *argument = parser_allocate(parser, sizeof(*argument), parser_site(parser));
        if (argument == NULL) {
            return false;
        }
        argument->span = parser->line->code;
        if (!parse_binding(parser, argument) || !append_argument(parser, node, argument)) {
            return false;
        }
        parser->line = parser->line->next;
        parser_skip_layout(parser);
    }
    if ((node->as.call.mode == AST_RETAINED_READ || node->as.call.mode == AST_CONSTANT_READ)
        && node->as.call.output_count == 0) {
        return parser_fail(parser, node->span, "A retained read needs at least one receiver", ">> receiver");
    }
    return true;
}
