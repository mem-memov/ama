#ifndef AMA_PARSER_ERROR_H
#define AMA_PARSER_ERROR_H

#include "diagnostics/record/diagnostic.h"

typedef enum {
    EXPECT_NONE,
    EXPECT_UTF8,
    EXPECT_LAYOUT,
    EXPECT_SECTION,
    EXPECT_NATIVE_LAYOUT,
    EXPECT_SECTION_ORDER,
    EXPECT_IMPLEMENTATION,
    EXPECT_ACCESS,
    EXPECT_CONSTANT_SOURCE,
    EXPECT_QUALIFIED_CONSTANT,
    EXPECT_OWNED_ACCESS,
    EXPECT_INSTANCE_ACCESS,
    EXPECT_DECLARATION_NAME,
    EXPECT_TYPE_LINE,
    EXPECT_TOP_DECLARATION,
    EXPECT_LOOP_CLOSE,
    EXPECT_NESTING,
    EXPECT_CONDITION_ADJACENCY,
    EXPECT_STATEMENT_TAIL,
    EXPECT_NAMESPACE_PATH,
    EXPECT_NEXT_STATEMENT,
    EXPECT_CALL_BINDING,
    EXPECT_NAMED_BINDING,
    EXPECT_BOUND_OPERAND,
    EXPECT_BINDING_STYLE,
    EXPECT_POSITIONAL_BINDING,
    EXPECT_ENTRY_BINDINGS,
    EXPECT_RESUME_BINDINGS,
    EXPECT_READ_BINDINGS,
    EXPECT_READ_RECEIVER,
    EXPECT_ABI_SPACE,
    EXPECT_C_IDENTIFIER,
    EXPECT_LINE_END,
    EXPECT_ABI_SYMBOL,
    EXPECT_ABI_NEW_SYMBOL,
    EXPECT_ABI_CONSTRUCTOR,
    EXPECT_ABI_DESTRUCTOR,
    EXPECT_COPY_SOURCE,
    EXPECT_RIGHT_BRACKET,
    EXPECT_ACCESS_END,
    EXPECT_COUNT
} ParserExpectation;

typedef struct {
    Diagnostic diagnostic;
    ParserExpectation expected;
} ParseError;

/* Stable grammar expectations for editor/checking consumers. Canonical display
 * text is derived from this key, not guessed from names or diagnostic wording. */
const char *parser_expected_text(ParserExpectation expectation);
bool parser_error_set(ParseError *error, SourceSpan span, const char *message,
    ParserExpectation expected);
void parser_error_from_layout(ParseError *error, const Diagnostic *diagnostic);

#endif
