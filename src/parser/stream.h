#ifndef AMA_PARSER_STREAM_H
#define AMA_PARSER_STREAM_H

#include "lexer/lexer.h"

typedef struct {
    Token* tokens;
    int token_count;
    int position;
} TokenStream;

/* The reader owns its token array and literal buffers, including on failure.
    Pass an empty stream, and destroy it after parsing on every path.
*/
bool read_token_stream(Lexer* lexer, TokenStream* stream);
void destroy_token_stream(TokenStream* stream);

/* Streams are nonempty and terminated by EOF; peeking past EOF is bounded. */

static inline Token* token_stream_peek(TokenStream* stream) {
    if (stream->position >= stream->token_count)
        return &stream->tokens[stream->token_count - 1];
    return &stream->tokens[stream->position];
}

static inline Token* token_stream_peek_next(TokenStream* stream) {
    if (stream->position + 1 >= stream->token_count)
        return &stream->tokens[stream->token_count - 1];
    return &stream->tokens[stream->position + 1];
}

static inline Token token_stream_advance(TokenStream* stream) {
    if (stream->position >= stream->token_count)
        return stream->tokens[stream->token_count - 1];
    return stream->tokens[stream->position++];
}

#endif
