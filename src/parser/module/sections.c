#include "sections.h"

static int annotation(const LayoutLine *line) {
    const char *markers[] = {"@=", "@<", "@>", "@!", "@C"};
    if (line->kind != LINE_CODE) {
        return -1;
    }
    for (int index = 0; index < 5; index++) {
        if (layout_text_is(line->code, markers[index])) {
            return index;
        }
    }
    return -1;
}

static bool active_file(const SourceLayout *layout) {
    for (const LayoutLine *line = layout->first; line != NULL; line = line->next) {
        int marker = annotation(line);
        if (line->indentation == 0 && (marker == 0 || marker >= 3)) {
            return true;
        }
    }
    return false;
}

static bool fail(ParseError *error, SourceSpan span, const char *message, ParserExpectation expected) {
    return parser_error_set(error, span, message, expected);
}

static ParsedSection *select_section(ModuleSections *sections, int marker) {
    return marker == 0 ? &sections->constants : marker == 1 ? &sections->parameters
        : marker == 2 ? &sections->results : &sections->implementation;
}

static bool read_sections(SourceLayout *layout, ModuleSections *sections, ParseError *error) {
    int previous = -1;
    ParsedSection *current = NULL;
    for (const LayoutLine *line = layout->first; line != NULL; line = line->next) {
        if (line->kind == LINE_BLANK || line->kind == LINE_COMMENT) {
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
        int marker = annotation(line);
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
    if (!active_file(layout)) {
        *output = sections;
        return true;
    }
    if (layout_validate(layout, false) != LAYOUT_OK) {
        parser_error_from_layout(error, &layout->diagnostic);
        return false;
    }
    if (!read_sections(layout, &sections, error)) {
        return false;
    }
    *output = sections;
    return true;
}
