#ifndef AMA_PARSER_STREAM_H
#define AMA_PARSER_STREAM_H

#include "lexer/token.h"

typedef struct {
    Token* data;
    int size;
    int pos;
} TokenStream;

/* Streams are nonempty and terminated by EOF; peeking past EOF is bounded. */

static inline Token* ts_peek(TokenStream* t) {
    if (t->pos >= t->size)
        return &t->data[t->size - 1];
    return &t->data[t->pos];
}

static inline Token* ts_peek2(TokenStream* t) {
    if (t->pos + 1 >= t->size)
        return &t->data[t->size - 1];
    return &t->data[t->pos + 1];
}

static inline Token ts_next(TokenStream* t) {
    if (t->pos >= t->size)
        return t->data[t->size - 1];
    return t->data[t->pos++];
}

#endif
