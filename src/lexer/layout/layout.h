/**
 * @file
 * @brief Expose physical lines and borrowed code/payload views for separate syntax stages.
 *
 * [SYN-01](../../../spec/language/source-and-names.md#syn-01)
 * Record exact nesting bytes; region-aware token scanning checks code joins and comment indentation.
 */
#ifndef AMA_LEXER_LAYOUT_H
#define AMA_LEXER_LAYOUT_H

#include "source/memory/arena.h"
#include "diagnostics/record/diagnostic.h"

/** Physical line categories; input literals retain distinct code and data view bounds. */
typedef enum {
    LINE_BLANK, LINE_COMMENT, LINE_CODE, LINE_LITERAL, LINE_INPUT_LITERAL
} LayoutKind;
/** Exact physical terminator kind; absent final newline is not fabricated. */
typedef enum { ENDING_NONE, ENDING_LF, ENDING_CRLF } LayoutEnding;
/** Arena-owned physical line node with original spans and semantic one-space indentation count. */
typedef struct LayoutLine {
    struct LayoutLine *next;
    SourceSpan physical;
    SourceSpan code;
    SourceSpan payload;
    size_t indentation;
    LayoutKind kind;
    LayoutEnding ending;
} LayoutLine;

/** Physical reading/validation outcomes, with exact borrowed diagnostic context on failure. */
typedef enum { LAYOUT_OK, LAYOUT_INVALID, LAYOUT_OUT_OF_MEMORY } LayoutStatus;
/** Owner of line nodes and a borrowed immutable source; dependent parsers must finish before destruction. */
typedef struct {
    SourceArena arena;
    const SourceText *source;
    LayoutLine *first;
    LayoutLine *last;
    Diagnostic diagnostic;
} SourceLayout;

/* Read once after init. All ranges borrow immutable source; nodes belong to
 * layout until destroy. Reading validates global UTF-8/TAB/line-ending rules.
 * Active code joins and structural-region checks use lexer/tokens independently. */
/** Initialize empty line storage; allocator authority outlives the layout.
 */
void layout_init(SourceLayout *layout, SourceAllocator allocator);
/** Release complete or failed-read line records. Source bytes remain borrowed and owned elsewhere.
 */
void layout_destroy(SourceLayout *layout);
/** Read physical Ama lines after global UTF-8/TAB/line-ending validation, retaining untouched payload/endings. Strict active grammar is separate. Partial nodes remain owned on failure.
 */
LayoutStatus layout_read(SourceLayout *layout, const SourceText *source);
/* ABI | is a binding symbol, never a literal-data boundary. */
/** Read the same physical line structure with literal classification disabled: ABI | is a marker, not a data boundary.
 */
LayoutStatus layout_read_abi(SourceLayout *layout, const SourceText *source);
/** Compare a valid borrowed span with exact NUL-terminated bytes, without normalization or source mutation.
 */
bool layout_text_is(SourceSpan span, const char *text);

#endif
