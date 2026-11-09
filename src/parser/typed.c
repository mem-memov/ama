#include "parser/internal.h"
#include "lexer/characters.h"
#include <string.h>

static bool valid_type_path(const char* text) {
    const unsigned char* byte = (const unsigned char*)text;
    while (*byte) {
        if (*byte++ != '.' || !is_identifier_start_character(*byte)) return false;
        byte++;
        while (is_identifier_character(*byte)) byte++;
    }
    return true;
}

static bool adjacent(Token left, Token right) {
    return left.line_number == right.line_number &&
        left.column_number + strlen(left.lexeme) == right.column_number;
}

static bool line_ended(StatementParser* parser, Token last) {
    Token* next = token_stream_peek(parser->stream);
    return next->type == TOKEN_END_OF_FILE || next->line_number != last.line_number ||
        parser_error(parser, next->line_number, "unexpected token after declaration name, type, or initializer");
}

static bool parse_storage(StatementParser* parser, Token name, AstStorage* storage, Token* last) {
    bool reference = false, list = false;
    *last = name;
    Token* next = token_stream_peek(parser->stream);
    if (next->type == TOKEN_AMPERSAND && next->line_number == name.line_number) {
        if (!adjacent(*last, *next))
            return parser_error(parser, next->line_number, "'&' must immediately follow the variable name");
        *last = token_stream_advance(parser->stream);
        reference = true;
        next = token_stream_peek(parser->stream);
    }
    if (next->type == TOKEN_LIST_OPEN && next->line_number == name.line_number) {
        if (!adjacent(*last, *next))
            return parser_error(parser, next->line_number, "'[]' must immediately follow the variable name");
        Token open = token_stream_advance(parser->stream);
        Token close = token_stream_advance(parser->stream);
        if (close.type != TOKEN_LIST_CLOSE || !adjacent(open, close))
            return parser_error(parser, close.line_number, "expected contiguous empty '[]' in declaration or source");
        *last = close;
        list = true;
    }
    *storage = reference ? (list ? AST_REFERENCE_LIST : AST_REFERENCE) : (list ? AST_OWNED_LIST : AST_OWNED);
    return line_ended(parser, *last);
}

static bool parse_initializer(StatementParser* parser, AstInitializer* initializer) {
    Token token = token_stream_advance(parser->stream);
    initializer->value.location = token_location(token);
    initializer->value.symbol_id = AST_UNRESOLVED_SYMBOL;
    if (token.type == TOKEN_LINE_LITERAL) {
        char* bytes = ast_allocate(parser->module, token.literal_length + 1, parser->error);
        if (!bytes) return false;
        memcpy(bytes, token.literal_bytes, token.literal_length);
        initializer->value.kind = AST_LINE_LITERAL_VALUE;
        initializer->value.literal_bytes = bytes;
        initializer->value.literal_length = token.literal_length;
        return true;
    }
    if (token.type == TOKEN_AMPERSAND) {
        Token name = token_stream_advance(parser->stream);
        if (name.type != TOKEN_IDENTIFIER || !adjacent(token, name))
            return parser_error(parser, name.line_number, "expected an adjacent owned name after '&'");
        initializer->acquires_reference = true;
        token = name;
    }
    if (token.type != TOKEN_IDENTIFIER)
        return parser_error(parser, token.line_number, "initializer must be a '|'-literal or a named value");
    initializer->value.kind = AST_NAME_VALUE;
    memcpy(initializer->value.text, token.lexeme, sizeof(initializer->value.text));
    Token last;
    if (!parse_storage(parser, token, &initializer->source_storage, &last)) return false;
    if (initializer->acquires_reference && initializer->source_storage != AST_OWNED)
        return parser_error(parser, last.line_number, "cannot acquire a reference to a reference or whole list");
    return true;
}

bool parse_typed_declaration(StatementParser* parser, Token name, uint32_t indentation,
    AstDeclarationKind kind, AstDeclaration** result) {
    AstDeclaration* declaration = ast_allocate(parser->module, sizeof(*declaration), parser->error);
    if (!declaration) return false;
    *result = declaration;
    declaration->is_typed = true;
    declaration->kind = kind;
    declaration->name = ast_name_from_token(name);
    Token last;
    if (!parse_storage(parser, name, &declaration->storage, &last)) return false;
    Token type = token_stream_advance(parser->stream);
    if (type.type != TOKEN_MODULE_PATH || !type.is_at_line_start || type.indentation != indentation)
        return parser_error(parser, type.line_number, "expected an unmodified module type path at indent %u", indentation);
    if (!valid_type_path(type.lexeme))
        return parser_error(parser, type.line_number, "invalid module type path '%s'", type.lexeme);
    if (!line_ended(parser, type)) return false;
    declaration->type_path.kind = AST_PATH_VALUE;
    declaration->type_path.location = token_location(type);
    declaration->type_path.symbol_id = AST_UNRESOLVED_SYMBOL;
    memcpy(declaration->type_path.text, type.lexeme, sizeof(declaration->type_path.text));
    while (token_stream_peek(parser->stream)->type != TOKEN_END_OF_FILE &&
        token_stream_peek(parser->stream)->indentation == indentation + 1) {
        AstInitializer* initializer = ast_allocate(parser->module, sizeof(*initializer), parser->error);
        if (!initializer || !parse_initializer(parser, initializer)) return false;
        if (declaration->last_initializer) declaration->last_initializer->next = initializer;
        else declaration->initializers = initializer;
        declaration->last_initializer = initializer;
    }
    return true;
}

bool parse_typed_declaration_section(Lexer* lexer, AstModule* module, AstDeclarationKind kind) {
    TokenStream stream = {0};
    bool success = read_token_stream(lexer, &stream);
    StatementParser parser = {.stream = &stream, .module_path = "<typed declarations>",
        .module = module, .error = lexer->error, .typed_declarations = true};
    while (success && token_stream_peek(&stream)->type != TOKEN_END_OF_FILE) {
        Token name = token_stream_advance(&stream);
        if (name.type != TOKEN_IDENTIFIER || !name.is_at_line_start || name.indentation != 0) {
            success = parser_error(&parser, name.line_number, "expected a typed variable name at indent 0");
            break;
        }
        AstDeclaration* declaration = NULL;
        success = parse_typed_declaration(&parser, name, 0, kind, &declaration);
        if (success) ast_append_declaration(module, declaration);
    }
    destroy_token_stream(&stream);
    return success;
}
