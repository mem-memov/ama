/**
 * @file
 * @brief Build allocation-free parser errors with exact sites and branch-specific help.
 *
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * First-error ownership belongs to the parser caller; no name lookup or heuristic repair is performed.
 */
#include "error.h"

/** Return immutable branch-specific help only for explicitly supported grammar keys; absent help remains null rather than a guessed repair.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04). */
static const char *explanation(ParserExpectation expected) {
    switch (expected) {
        case EXPECT_RIGHT_BRACKET:
            return "Close the indexed access with ]";
        case EXPECT_TYPE_LINE:
            return "Put the module type on its own line at the name's indentation";
        case EXPECT_CONDITION_ADJACENCY:
            return "Keep the condition and ?/{ marker adjacent; comment lines are allowed";
        case EXPECT_STATEMENT_TAIL:
            return "Use ! to execute, % to resume, or indented >> bindings to read results";
        case EXPECT_COPY_SOURCE:
            return "Initialize a typed declaration with | first, then use its name here";
        case EXPECT_ENTRY_BINDINGS:
            return "Put all < inputs before > outputs; use an unmarked target for >> reads";
        case EXPECT_RESUME_BINDINGS:
            return "Resumption preserves inputs and accepts only > result bindings";
        case EXPECT_READ_BINDINGS:
        case EXPECT_READ_RECEIVER:
            return "Omit !/% and supply at least one >> receiver for a retained read";
        case EXPECT_BINDING_STYLE:
            return "Use either named labels throughout or positional bindings throughout";
        case EXPECT_NESTING:
            return "Each nested body adds exactly one ASCII space";
        case EXPECT_ABI_NEW_SYMBOL:
            return "Remove the duplicate record; each ABI symbol has one binding";
        default:
            return NULL;
    }
}

bool parser_error_set(ParseError *error, SourceSpan span, const char *message,
    ParserExpectation expected) {
    *error = (ParseError){0};
    error->expected = expected;
    error->diagnostic.code = DIAGNOSTIC_SYNTAX;
    error->diagnostic.primary.span = span;
    error->diagnostic.message = message;
    error->diagnostic.expected = parser_expected_text(expected);
    error->diagnostic.help = explanation(expected);
    return false;
}

void parser_error_from_layout(ParseError *error, const Diagnostic *diagnostic) {
    error->diagnostic = *diagnostic;
    error->expected = diagnostic->code == DIAGNOSTIC_INVALID_UTF8 ? EXPECT_UTF8
        : diagnostic->code == DIAGNOSTIC_OUT_OF_MEMORY ? EXPECT_NONE : EXPECT_LAYOUT;
    /* Keep the originating check's narrower expectation; the enum is a broad
     * machine category, not permission to replace its exact explanation. */
    if (error->diagnostic.expected == NULL) {
        error->diagnostic.expected = parser_expected_text(error->expected);
    }
}
