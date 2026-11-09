#include "parser/stream.h"
#include <limits.h>
#include <stdlib.h>

bool read_token_stream(Lexer* lexer, TokenStream* stream) {
    int capacity = 0;
    for (;;) {
        Token token = get_next_token(lexer);
        if (token.type == TOKEN_ERROR) { destroy_token(&token); return false; }
        if (stream->token_count == capacity) {
            if (capacity > INT_MAX / 2 || (size_t)(capacity ? capacity * 2 : 64) > SIZE_MAX / sizeof(Token)) {
                destroy_token(&token);
                return error_set(lexer->error, ERROR_LIMIT, token.line_number, "too many tokens");
            }
            int next_capacity = capacity ? capacity * 2 : 64;
            Token* tokens = realloc(stream->tokens, sizeof(Token) * (size_t)next_capacity);
            if (!tokens) {
                destroy_token(&token);
                return error_set(lexer->error, ERROR_MEMORY, token.line_number, "out of memory (tokens)");
            }
            stream->tokens = tokens;
            capacity = next_capacity;
        }
        stream->tokens[stream->token_count++] = token;
        if (token.type == TOKEN_END_OF_FILE) return true;
    }
}

void destroy_token_stream(TokenStream* stream) {
    for (int index = 0; index < stream->token_count; index++) destroy_token(&stream->tokens[index]);
    free(stream->tokens);
    *stream = (TokenStream){0};
}
