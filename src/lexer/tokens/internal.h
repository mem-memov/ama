/**
 * @file
 * @brief Separate lexical recognition, spacing relations and checked cursor publication.
 * [SYN-06](../../../spec/language/source-and-names.md#syn-06) makes token boundaries
 * depend on original characters, never declaration inventories.
 */
#ifndef AMA_LEXER_TOKENS_INTERNAL_H
#define AMA_LEXER_TOKENS_INTERNAL_H
#include "tokens.h"
/** Record one exact borrowed diagnostic site and close this cursor after failure. Component contract: [SYN-06](../../../spec/language/source-and-names.md#syn-06). */
bool lex_failure(LexCursor *, size_t, const char *, const char *);
/** Recognize a maximal complete name or longest fixed punctuation without mutating the cursor. Component contract: [SYN-06](../../../spec/language/source-and-names.md#syn-06). */
bool lex_recognize(LexCursor *, size_t, LexToken *);
/** Validate the current/previous token's compact or separated join. Component contract: [SYN-06](../../../spec/language/source-and-names.md#syn-06). */
bool lex_boundary(LexCursor *, const LexToken *);
#endif
