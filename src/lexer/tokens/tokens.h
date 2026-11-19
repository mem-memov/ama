/**
 * @file
 * @brief Scan exact complete-name and punctuation tokens within an explicitly supplied structural region.
 * [SYN-06](../../../spec/language/source-and-names.md#syn-06) fixes longest compound
 * tokens and compact joins; [SYN-02](../../../spec/language/source-and-names.md#syn-02)
 * requires comment indentation to match the caller's grammar region.
 */
#ifndef AMA_LEXER_TOKENS_H
#define AMA_LEXER_TOKENS_H
#include "lexer/layout/layout.h"
/** Lexical categories contain no name-binding or declared-type decisions. Component contract: [SYN-06](../../../spec/language/source-and-names.md#syn-06), [SYN-02](../../../spec/language/source-and-names.md#syn-02). */
typedef enum {
    LEX_NAME, LEX_LITERAL, LEX_CONSTANTS, LEX_PARAMETERS, LEX_RESULTS, LEX_AMA, LEX_NATIVE,
    LEX_STATIC_MODULE,
    LEX_READ, LEX_LIST, LEX_REFERENCE_LIST, LEX_REFERENCE_INDEX, LEX_STANDARD_ROOT,
    LEX_DOT, LEX_REFERENCE, LEX_INDEX_OPEN, LEX_INDEX_CLOSE, LEX_EXECUTE, LEX_CONTINUATION,
    LEX_INPUT, LEX_OUTPUT, LEX_QUESTION, LEX_COLON, LEX_LOOP_OPEN, LEX_LOOP_CLOSE, LEX_BREAK,
    LEX_STATIC_PREFIX
} LexKind;
/** Borrowed original token span and optional untouched literal payload; separator records an actual single code space. Component contract: [SYN-06](../../../spec/language/source-and-names.md#syn-06), [SYN-02](../../../spec/language/source-and-names.md#syn-02). */
typedef struct {
    LexKind kind;
    SourceSpan text;
    SourceSpan payload;
    bool separator;
} LexToken;
/** Allocation-free line cursor. Layout/source owners outlive scanning and every returned view. Component contract: [SYN-06](../../../spec/language/source-and-names.md#syn-06), [SYN-02](../../../spec/language/source-and-names.md#syn-02). */
typedef struct {
    const LayoutLine *line;
    size_t offset;
    LexToken previous;
    Diagnostic error;
    bool has_previous;
    bool reference_prefix;
    bool failed;
} LexCursor;
/** End is distinct from malformed code; invalid scans never publish an output token. Component contract: [SYN-06](../../../spec/language/source-and-names.md#syn-06), [SYN-02](../../../spec/language/source-and-names.md#syn-02). */
typedef enum { LEX_TOKEN, LEX_END, LEX_INVALID } LexStatus;
/** Begin a line already read by layout_read, at the grammar region's exact indentation.
 * Empty physical lines ignore depth; comment-only lines obey it. Reject spaces-only
 * lines and nonliteral trailing code spaces. No statement/nesting intent is guessed.
 * Component contract: [SYN-06](../../../spec/language/source-and-names.md#syn-06), [SYN-02](../../../spec/language/source-and-names.md#syn-02). */
bool lex_line_begin(LexCursor *, const LayoutLine *, size_t);
/** Read one longest token and validate its exact spacing joins without allocation.
 * Complete names combine XID words separated by single spaces; literals keep raw
 * payloads. Structural grammar/name binding remains the parser's responsibility.
 * Component contract: [SYN-06](../../../spec/language/source-and-names.md#syn-06), [SYN-02](../../../spec/language/source-and-names.md#syn-02). */
LexStatus lex_token_next(LexCursor *, LexToken *);
/** Scan a maximal complete name from a bounded cursor using the same predicate as tokens.
 * Failure changes neither cursor nor name; punctuation/separators remain outside
 * the returned span. No name lookup or shorter-name alternative is attempted.
 * Component contract: [SYN-06](../../../spec/language/source-and-names.md#syn-06), [SYN-02](../../../spec/language/source-and-names.md#syn-02). */
bool lex_name(SourceSpan, size_t *, SourceSpan *);
#endif
