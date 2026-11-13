/**
 * @file
 * @brief Separate lexical recognition, spacing relations and checked cursor publication.
 * [SYN-06](../../../spec/language/source-and-names.md#syn-06) makes token boundaries
 * depend on original characters, never declaration inventories.
 */
#ifndef AMA_LEXER_TOKENS_INTERNAL_H
#define AMA_LEXER_TOKENS_INTERNAL_H
#include "tokens.h"
/** Record one exact borrowed diagnostic site and close this cursor after failure. */
bool lex_failure(LexCursor *, size_t, const char *, const char *);
/** Recognize a maximal complete name or longest fixed punctuation without mutating the cursor. */
bool lex_recognize(LexCursor *, size_t, LexToken *);
/** Validate the current/previous token's compact or separated join and classify a standalone ampersand's role. */
bool lex_boundary(LexCursor *, const LexToken *, bool *);
#endif
