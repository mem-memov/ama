#include "lexer/lexer.h"
#include "lexer/characters.h"
#include <stdlib.h>
#include <string.h>

static int read_byte(Lexer* lexer) {
    if (ftell(lexer->source_file) >= lexer->section_end_offset) return EOF;
    return fgetc(lexer->source_file);
}

static void return_byte_to_input(Lexer* lexer, int character) {
    (void)lexer;
    if (character != EOF) ungetc(character, lexer->source_file);
}

static int read_character(Lexer* lexer) {
    int character = read_byte(lexer);
    if (character == EOF) return EOF;
    if (character == '\r') {
        int next_character = read_byte(lexer);
        if (next_character == '\n') return '\n';
        return_byte_to_input(lexer, next_character);
        error_set(lexer->error, ERROR_INVALID, lexer->line_number,
            "stray \\r (not followed by \\n)");
        return EOF;
    }
    return character;
}

bool init_lexer(Lexer* lexer, FILE* source_file, long section_start_offset, long section_end_offset,
    uint32_t start_line, Error* error) {
    lexer->error = error;
    lexer->source_file = source_file;
    lexer->section_end_offset = section_end_offset;
    lexer->line_number = start_line;
    lexer->is_at_line_start = 1;
    lexer->indentation = 0;
    lexer->has_preceding_blank_line = 0;
    if (fseek(source_file, section_start_offset, SEEK_SET) != 0)
        return error_set(error, ERROR_IO, start_line, "cannot seek source file");
    return true;
}

/* Return the first significant byte, preserving blank-line/comment semantics. */
static int skip_whitespace_and_comments(Lexer* lexer) {
    for (;;) {
        int character = read_character(lexer);
        if (character == '\n') {
            if (lexer->is_at_line_start)
                lexer->has_preceding_blank_line = 1;
            lexer->line_number++;
            lexer->indentation = 0;
            lexer->is_at_line_start = 1;
        } else if (character == '#') {
            while ((character = read_character(lexer)) != '\n' && character != EOF)
                ;
            if (character == '\n')
                lexer->line_number++;
            lexer->indentation = 0;
            lexer->is_at_line_start = 1;
        } else if (character == ' ') {
            if (lexer->is_at_line_start)
                lexer->indentation++;
        } else {
            return character;
        }
    }
}

static Token begin_token(Lexer* lexer) {
    Token token = {0};
    token.line_number = lexer->line_number;
    token.indentation = lexer->indentation;
    token.is_at_line_start = lexer->is_at_line_start;
    token.has_preceding_blank_line = lexer->has_preceding_blank_line;
    lexer->is_at_line_start = 0;
    lexer->has_preceding_blank_line = 0;
    return token;
}

static int read_punctuation(Token* token, int character) {
    switch (character) {
    case '<': token->type = TOKEN_PARAMETER_ARROW; break;
    case '>': token->type = TOKEN_RESULT_ARROW; break;
    case '?': token->type = TOKEN_QUESTION_MARK; break;
    case ':': token->type = TOKEN_COLON; break;
    case '{': token->type = TOKEN_LOOP_OPEN; break;
    case '}': token->type = TOKEN_LOOP_CLOSE; break;
    case '!': token->type = TOKEN_BREAK; break;
    default: return 0;
    }
    token->lexeme[0] = (char)character;
    token->lexeme[1] = '\0';
    return 1;
}

/* The three word forms share length handling but retain their own diagnostics. */
static int is_token_continuation(TokenType type, int character) {
    if (type == TOKEN_LITERAL)
        return is_decimal_digit(character);
    return is_identifier_character(character) || (type == TOKEN_MODULE_PATH && character == '.');
}

static void report_token_length_error(Lexer* lexer, TokenType type, uint32_t line_number) {
    error_set(lexer->error, ERROR_LIMIT, line_number, "%s too long at line %u (max %d %s)",
        type == TOKEN_LITERAL ? "literal" : type == TOKEN_MODULE_PATH ? "path" : "identifier",
        line_number, MAX_TOKEN_TEXT_LENGTH - 1, type == TOKEN_LITERAL ? "digits" : "bytes");
}

static Token read_word(Lexer* lexer, Token token, int first_character, TokenType type) {
    int length = 0;
    token.type = type;
    token.lexeme[length++] = (char)first_character;
    for (;;) {
        int character = read_character(lexer);
        if (character == EOF)
            break;
        if (!is_token_continuation(type, character)) {
            return_byte_to_input(lexer, character);
            break;
        }
        if (length >= MAX_TOKEN_TEXT_LENGTH - 1) {
            report_token_length_error(lexer, type, token.line_number);
            token.type = TOKEN_ERROR;
            return token;
        }
        token.lexeme[length++] = (char)character;
    }
    token.lexeme[length] = '\0';
    if (lexer->error->error_code != ERROR_NONE) token.type = TOKEN_ERROR;
    return token;
}

static Token read_identifier(Lexer* lexer, Token token, int first_character) {
    return read_word(lexer, token, first_character, TOKEN_IDENTIFIER);
}

static Token read_literal(Lexer* lexer, Token token, int first_character) {
    return read_word(lexer, token, first_character, TOKEN_LITERAL);
}

static Token read_path(Lexer* lexer, Token token) {
    return read_word(lexer, token, '.', TOKEN_MODULE_PATH);
}

Token get_next_token(Lexer* lexer) {
    int character = skip_whitespace_and_comments(lexer);
    if (ferror(lexer->source_file) && lexer->error->error_code == ERROR_NONE)
        error_set(lexer->error, ERROR_IO, lexer->line_number, "cannot read source file");
    if (lexer->error->error_code != ERROR_NONE) {
        Token failure = {TOKEN_ERROR, "ERROR", lexer->line_number, lexer->indentation, 0, 0};
        return failure;
    }
    if (character == EOF) {
        Token end_of_file_token = {TOKEN_END_OF_FILE, "EOF", lexer->line_number,
                        lexer->indentation, 1, lexer->has_preceding_blank_line};
        return end_of_file_token;
    }
    Token token = begin_token(lexer);
    if (read_punctuation(&token, character))
        return token;
    if (character == '.')
        return read_path(lexer, token);
    if (is_decimal_digit(character))
        return read_literal(lexer, token, character);
    if (is_identifier_start_character(character))
        return read_identifier(lexer, token, character);

    error_set(lexer->error, ERROR_INVALID, token.line_number, "unexpected byte 0x%02X ('%c') at line %u",
        (unsigned char)character, (character >= 0x20 && character < 0x7F) ? character : '?', token.line_number);
    Token error = {TOKEN_ERROR, "ERROR", token.line_number, token.indentation, 0, 0};
    return error;
}
