/**
 * @file
 * @brief Map every retained grammar-expectation key to fixed display text.
 *
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * Compile-time table cardinality prevents missing keys; current grammar extensions remain explicit TODO tasks.
 * [REF-01](../../../spec/language/values-and-references.md#ref-01) requires the
 * acquisition hint to describe an owned operand separated by one ASCII space.
 */
#include "error.h"

const char *parser_expected_text(ParserExpectation expectation) {
    static const char *const descriptions[] = {
        NULL,
        "well-formed UTF-8",
        "LF or CRLF and ASCII code indentation",
        "a section annotation",
        "layout after @C",
        "ordinary @=, @<, @>, then @! or @C; constants-only @=; or exclusive @...",
        "@! or @C",
        "a complete Unicode name with compact reference, list or named-index suffixes",
        "a complete module-path operand, optionally ending in []",
        "an owned instance access separated from & by one ASCII space",
        "a scalar owned or reference access",
        "name, name&, name[] or name&[]",
        "a simple module path or Type placeholder at the declaration's indentation",
        "a declaration at column one",
        "} at the condition's indentation",
        "exactly one additional space for a nested body",
        "an owned access followed by ? or { without a blank line",
        "a type line, an indented copy source, an adjacent condition marker, !, %, or >> requests",
        "a simple Type namespace without ! or %, optionally selecting one named export",
        "a subsequent statement",
        "< source or < |payload, > receiver, or >> receiver",
        "name < source or name < |payload, name > receiver, or name >> receiver",
        "a space and source/receiver after the marker",
        "one binding style throughout a call",
        "inputs < before outputs >",
        "outputs > only",
        "retained outputs >> only",
        ">> receiver",
        "one or more ASCII spaces",
        "one module-local C function identifier",
        "end of line or a comment",
        "+, -, |, =, ?, or []",
        "a previously unbound ABI symbol",
        "+ default-construction binding",
        "- destruction binding",
        "a named value/reference, qualified constant, or explicit copy source",
        "]",
        "end of access",
        "a complete module path or single-dot static name",
        "a complete single-dot label, < or >, and its required operand",
        "a static result name followed by a same-level type expression",
    };
    _Static_assert(sizeof(descriptions) / sizeof(descriptions[0]) == EXPECT_COUNT,
        "Every grammar expectation needs a description");
    return expectation >= 0 && expectation < EXPECT_COUNT ? descriptions[expectation] : NULL;
}
