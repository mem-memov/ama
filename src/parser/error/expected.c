/**
 * @file
 * @brief Map every retained grammar-expectation key to fixed display text.
 *
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * Compile-time table cardinality prevents missing keys; current grammar extensions remain explicit TODO tasks.
 */
#include "error.h"

const char *parser_expected_text(ParserExpectation expectation) {
    static const char *const descriptions[] = {
        NULL,
        "well-formed UTF-8",
        "LF or CRLF and ASCII code indentation",
        "a section annotation",
        "layout after @C",
        "optional @=, @<, @>, followed by @! or @C; or constants-only @=",
        "@! or @C",
        "a complete Unicode name with compact reference, list or named-index suffixes",
        "a Project or Standard qualified constant, optionally ending in []",
        "a module path followed by a dot and a complete constant name",
        "an adjoining owned instance access after &",
        "a scalar owned or reference access",
        "name, name&, name[] or name&[]",
        "a module path at the declaration's indentation",
        "a declaration at column one",
        "} at the condition's indentation",
        "exactly one additional space for a nested body",
        "an owned access followed by ? or { without a blank line",
        "a type line, an indented copy source, an adjacent condition marker, !, %, or >> requests",
        "a module path without ! or %",
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
        "a complete module path or three-dot static name",
        "a three-dot named or positional static < or > binding",
        "a three-dot formal name with an optional nonempty signature",
        "a static result name followed by a same-level type expression",
    };
    _Static_assert(sizeof(descriptions) / sizeof(descriptions[0]) == EXPECT_COUNT,
        "Every grammar expectation needs a description");
    return expectation >= 0 && expectation < EXPECT_COUNT ? descriptions[expectation] : NULL;
}
