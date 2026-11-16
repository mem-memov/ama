/**
 * @file
 * @brief Recognize genuine activation markers and monotonically ordered section intervals.
 *
 * [SYN-03](../../../spec/language/source-and-names.md#syn-03)
 * Comments/literals do not activate modules; global byte checks precede activation.
 * The module parser consumes these exact intervals when assembling section tokens.
 */
#include "internal.h"

/** Publish an exact source-mapped section error using the shared parser diagnostic constructor.
 * Component contract: [SYN-03](../../../spec/language/source-and-names.md#syn-03). */
static bool fail(ParseError *error, SourceSpan span, const char *message, ParserExpectation expected) {
    return parser_error_set(error, span, message, expected);
}

/** Return the local section record for an already recognized marker; `@!`/`@C` share implementation storage.
 * Component contract: [SYN-03](../../../spec/language/source-and-names.md#syn-03). */
static ParsedSection *select_section(ModuleSections *sections, int marker) {
    return marker == 0 ? &sections->constants : marker == 1 ? &sections->parameters
        : marker == 2 ? &sections->results : &sections->implementation;
}

/** Maintain strictly increasing section rank, assigning each nonmarker line to its current interval. Partial local intervals are unpublished on failure.
 * Component contract: [SYN-03](../../../spec/language/source-and-names.md#syn-03). */
static bool read_sections(SourceLayout *layout, ModuleSections *sections, ParseError *error) {
    int previous = -1;
    ParsedSection *current = NULL;
    for (const LayoutLine *line = layout->first; line != NULL; line = line->next) {
        if (line->kind == LINE_BLANK || line->kind == LINE_COMMENT) {
            if (line->kind == LINE_COMMENT && (current == NULL || sections->kind == MODULE_NATIVE)) {
                LexCursor cursor;
                if (!lex_line_begin(&cursor, line, 0)) {
                    parser_error_from_layout(error, &cursor.error);
                    return false;
                }
            }
            continue;
        }
        bool annotation_line = line->kind == LINE_CODE
            && source_bytes(line->code.source)[line->code.start] == '@';
        if (!annotation_line) {
            if (current == NULL || sections->kind == MODULE_NATIVE) {
                return fail(error, line->code, "Unexpected content outside an Ama section",
                    current == NULL ? EXPECT_SECTION : EXPECT_NATIVE_LAYOUT);
            }
            continue;
        }
        int marker = parser_annotation(line);
        int rank = marker == 4 ? 3 : marker;
        if (marker < 0 || line->indentation != 0 || rank <= previous) {
            return fail(error, line->code, "Unknown, repeated, indented or out-of-order annotation",
                EXPECT_SECTION_ORDER);
        }
        if (current != NULL) {
            current->end = line;
        }
        current = select_section(sections, marker);
        *current = (ParsedSection){line, line->next, NULL};
        previous = rank;
        sections->kind = marker == 3 ? MODULE_AMA : marker == 4 ? MODULE_NATIVE : MODULE_CONSTANTS;
    }
    if (previous == 1 || previous == 2) {
        size_t end = source_length(layout->source);
        return fail(error, (SourceSpan){layout->source, end, end},
            "Interface sections require an implementation selector", EXPECT_IMPLEMENTATION);
    }
    return true;
}

bool parser_sections(SourceLayout *layout, ModuleSections *output, ParseError *error) {
    ModuleSections sections = {0};
    bool active;
    if (!parser_module_classify(layout, &active, error)) { return false; }
    if (!active) {
        *output = sections;
        return true;
    }
    if (!read_sections(layout, &sections, error)) {
        return false;
    }
    *output = sections;
    return true;
}
