#ifndef AMA_LEXER_LAYOUT_H
#define AMA_LEXER_LAYOUT_H

#include "source/memory/arena.h"
#include "diagnostics/record/diagnostic.h"

typedef enum { LINE_BLANK, LINE_COMMENT, LINE_CODE, LINE_LITERAL } LayoutKind;
typedef enum { ENDING_NONE, ENDING_LF, ENDING_CRLF } LayoutEnding;
typedef struct LayoutLine {
    struct LayoutLine *next;
    SourceSpan physical;
    SourceSpan code;
    SourceSpan payload;
    size_t indentation;
    LayoutKind kind;
    LayoutEnding ending;
} LayoutLine;

typedef enum { LAYOUT_OK, LAYOUT_INVALID, LAYOUT_OUT_OF_MEMORY } LayoutStatus;
typedef struct {
    SourceArena arena;
    const SourceText *source;
    LayoutLine *first;
    LayoutLine *last;
    Diagnostic diagnostic;
} SourceLayout;

/* Read once after init. All ranges borrow immutable source; nodes belong to
 * layout until destroy. Reading validates UTF-8, not active-file grammar.
 * Strict whitespace checking is separate so unfinished drafts remain drafts. */
void layout_init(SourceLayout *layout, SourceAllocator allocator);
void layout_destroy(SourceLayout *layout);
LayoutStatus layout_read(SourceLayout *layout, const SourceText *source);
LayoutStatus layout_validate(SourceLayout *layout, bool native_abi);
bool layout_text_is(SourceSpan span, const char *text);

#endif
