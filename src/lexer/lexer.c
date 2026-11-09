#include "lexer/lexer.h"
#include "lexer/characters.h"
#include <stdlib.h>
#include <string.h>

static int read_byte(Lexer* lexer) {
    if (ftell(lexer->source_file) >= lexer->section_end_offset) return EOF;
    return fgetc(lexer->source_file);
}

static void return_byte_to_input(Lexer* lexer, int character) {
    if (character != EOF) {
        ungetc(character, lexer->source_file);
        lexer->column_number--;
    }
}

static int read_character(Lexer* lexer) {
    int character = read_byte(lexer);
    if (character == EOF) return EOF;
    if (character == '\r') {
        int next_character = read_byte(lexer);
        if (next_character == '\n') {
            lexer->column_number++;
            return '\n';
        }
        if (next_character != EOF) ungetc(next_character, lexer->source_file);
        error_set(lexer->error, ERROR_INVALID, lexer->line_number,
            "stray \\r (not followed by \\n)");
        return EOF;
    }
    lexer->column_number++;
    return character;
}

bool init_lexer(Lexer* lexer, FILE* source_file, const char* source_path,
    long section_start_offset, long section_end_offset, uint32_t start_line, Error* error) {
    lexer->error = error;
    lexer->source_file = source_file;
    lexer->source_path = source_path;
    lexer->column_number = 1;
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
            lexer->column_number = 1;
            lexer->is_at_line_start = 1;
        } else if (character == '#') {
            while ((character = read_character(lexer)) != '\n' && character != EOF)
                ;
            if (character == '\n') {
                lexer->line_number++;
                lexer->column_number = 1;
            }
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
    token.source_path = lexer->source_path;
    token.column_number = lexer->column_number - 1;
    token.indentation = lexer->indentation;
    token.is_at_line_start = lexer->is_at_line_start;
    token.has_preceding_blank_line = lexer->has_preceding_blank_line;
    lexer->is_at_line_start = 0;
    lexer->has_preceding_blank_line = 0;
    return token;
}

static int read_punctuation(Lexer* lexer, Token* token, int character) {
    switch (character) {
    case '<': token->type = TOKEN_PARAMETER_ARROW; break;
    case '>': {
        int next = read_character(lexer);
        if (next == '>') {
            token->type = TOKEN_RETAINED_RESULT_ARROW;
            strcpy(token->lexeme, ">>");
            return 1;
        }
        return_byte_to_input(lexer, next);
        token->type = TOKEN_RESULT_ARROW;
        break;
    }
    case '?': token->type = TOKEN_QUESTION_MARK; break;
    case ':': token->type = TOKEN_COLON; break;
    case '{': token->type = TOKEN_LOOP_OPEN; break;
    case '}': token->type = TOKEN_LOOP_CLOSE; break;
    case '!': token->type = TOKEN_CALL_MARKER; break;
    case '^': token->type = TOKEN_BREAK; break;
    case '&': token->type = TOKEN_AMPERSAND; break;
    case '[': token->type = TOKEN_LIST_OPEN; break;
    case ']': token->type = TOKEN_LIST_CLOSE; break;
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

void destroy_token(Token* token) {
    free(token->literal_bytes);
    token->literal_bytes = NULL;
    token->literal_length = 0;
}

static Token read_line_literal(Lexer* lexer, Token token) {
    token.type = TOKEN_LINE_LITERAL;
    strcpy(token.lexeme, "|");
    size_t capacity = 64;
    token.literal_bytes = malloc(capacity);
    if (!token.literal_bytes) {
        error_set(lexer->error, ERROR_MEMORY, token.line_number, "out of memory (line literal)");
        token.type = TOKEN_ERROR;
        return token;
    }
    for (;;) {
        int character = read_character(lexer);
        if (character == EOF) break;
        if (character == '\n') {
            return_byte_to_input(lexer, character);
            break;
        }
        if (token.literal_length == capacity - 1) {
            if (capacity > SIZE_MAX / 2) {
                error_set(lexer->error, ERROR_LIMIT, token.line_number, "line literal is too long");
                break;
            }
            capacity *= 2;
            char* bytes = realloc(token.literal_bytes, capacity);
            if (!bytes) {
                error_set(lexer->error, ERROR_MEMORY, token.line_number, "out of memory (line literal)");
                break;
            }
            token.literal_bytes = bytes;
        }
        token.literal_bytes[token.literal_length++] = (char)character;
    }
    if (lexer->error->error_code != ERROR_NONE) {
        destroy_token(&token);
        token.type = TOKEN_ERROR;
    } else {
        token.literal_bytes[token.literal_length] = '\0';
    }
    return token;
}

Token get_next_token(Lexer* lexer) {
    int character = skip_whitespace_and_comments(lexer);
    if (ferror(lexer->source_file) && lexer->error->error_code == ERROR_NONE)
        error_set(lexer->error, ERROR_IO, lexer->line_number, "cannot read source file");
    if (lexer->error->error_code != ERROR_NONE) {
        Token failure = {.type = TOKEN_ERROR, .lexeme = "ERROR",
            .line_number = lexer->line_number, .indentation = lexer->indentation,
            .source_path = lexer->source_path, .column_number = lexer->column_number};
        return failure;
    }
    if (character == EOF) {
        Token end_of_file_token = {.type = TOKEN_END_OF_FILE, .lexeme = "EOF",
            .line_number = lexer->line_number, .indentation = lexer->indentation,
            .is_at_line_start = 1, .has_preceding_blank_line = lexer->has_preceding_blank_line,
            .source_path = lexer->source_path, .column_number = lexer->column_number};
        return end_of_file_token;
    }
    Token token = begin_token(lexer);
    if (character == '|' && token.is_at_line_start)
        return read_line_literal(lexer, token);
    if (read_punctuation(lexer, &token, character)) {
        if (lexer->error->error_code != ERROR_NONE) token.type = TOKEN_ERROR;
        return token;
    }
    if (character == '.')
        return read_path(lexer, token);
    if (is_decimal_digit(character))
        return read_literal(lexer, token, character);
    if (is_identifier_start_character(character))
        return read_identifier(lexer, token, character);

    error_set(lexer->error, ERROR_INVALID, token.line_number, "unexpected byte 0x%02X ('%c') at line %u",
        (unsigned char)character, (character >= 0x20 && character < 0x7F) ? character : '?', token.line_number);
    Token error = token;
    error.type = TOKEN_ERROR;
    return error;
}
