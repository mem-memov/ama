#include "lexer/sections.h"
#include <stdlib.h>
#include <string.h>

typedef enum {
    SECTION_NONE,
    SECTION_PARAMETERS,
    SECTION_RESULTS,
    SECTION_CODE
} SectionKind;

static bool validate_line(const char* text, size_t length, uint32_t line_number, Error* error) {
    for (size_t index = 0; index < length; index++) {
        if (text[index] == '\t')
            return error_set(error, ERROR_INVALID, line_number,
                "tab character is not allowed; use spaces for indentation");
        if (text[index] == '\r' && (index + 1 == length || text[index + 1] != '\n'))
            return error_set(error, ERROR_INVALID, line_number,
                "stray \\r (not followed by \\n)");
    }
    return true;
}

/* Strip comments and trailing spaces; retain the pointer for indentation checks. */
static char* line_content(char* text) {
    text[strcspn(text, "#\r\n")] = '\0';
    size_t length = strlen(text);
    while (length && text[length - 1] == ' ') text[--length] = '\0';
    while (*text == ' ') text++;
    return text;
}

static SectionKind annotation_kind(const char* annotation) {
    if (!strcmp(annotation, "@<")) return SECTION_PARAMETERS;
    if (!strcmp(annotation, "@>")) return SECTION_RESULTS;
    if (!strcmp(annotation, "@Ama") || !strcmp(annotation, "@C")) return SECTION_CODE;
    return SECTION_NONE;
}

static void begin_section(
    ModuleSections* sections,
    SectionKind kind,
    long offset,
    uint32_t line_number
) {
    switch (kind) {
    case SECTION_PARAMETERS:
        sections->parameters_start = offset;
        sections->parameters_line = line_number;
        break;
    case SECTION_RESULTS:
        sections->results_start = offset;
        sections->results_line = line_number;
        break;
    case SECTION_CODE:
        sections->code_start = offset;
        sections->code_line = line_number;
        break;
    case SECTION_NONE:
        break;
    }
}

static void end_section(ModuleSections* sections, SectionKind kind, long offset) {
    switch (kind) {
    case SECTION_PARAMETERS:
        sections->parameters_end = offset;
        break;
    case SECTION_RESULTS:
        sections->results_end = offset;
        break;
    case SECTION_CODE:
        sections->code_end = offset;
        break;
    case SECTION_NONE:
        break;
    }
}

static bool read_annotation(
    ModuleSections* sections,
    SectionKind* current_section,
    const char* annotation,
    bool is_at_line_start,
    long line_offset,
    long next_line_offset,
    uint32_t line_number,
    Error* error
) {
    if (!is_at_line_start)
        return error_set(error, ERROR_INVALID, line_number,
            "section annotations must start at indentation 0");
    SectionKind kind = annotation_kind(annotation);
    if (kind == SECTION_NONE)
        return error_set(error, ERROR_INVALID, line_number,
            "unknown section annotation '%s'; expected @<, @>, @Ama, or @C", annotation);
    if (kind <= *current_section)
        return error_set(error, ERROR_INVALID, line_number,
            "duplicate or out-of-order section annotation '%s'", annotation);
    end_section(sections, *current_section, line_offset);
    begin_section(sections, kind, next_line_offset, line_number + 1);
    if (kind == SECTION_CODE)
        sections->implementation = !strcmp(annotation, "@C")
            ? MODULE_C_IMPLEMENTATION : MODULE_AMA_IMPLEMENTATION;
    *current_section = kind;
    return true;
}

bool split_module_sections(FILE* source_file, ModuleSections* sections, Error* error) {
    /* Missing declaration sections are empty lexer ranges. */
    *sections = (ModuleSections){.parameters_line = 1, .results_line = 1, .code_line = 1};
    char* text = NULL;
    size_t capacity = 0;
    ssize_t length;
    long line_offset = ftell(source_file);
    uint32_t line_number = 1;
    SectionKind current_section = SECTION_NONE;
    bool success = true;
    if (line_offset < 0)
        return error_set(error, ERROR_IO, 0, "cannot locate source section");
    while ((length = getline(&text, &capacity, source_file)) != -1) {
        long next_line_offset = ftell(source_file);
        if (next_line_offset < 0) {
            success = error_set(error, ERROR_IO, line_number, "cannot locate source section");
            break;
        }
        if (!validate_line(text, (size_t)length, line_number, error)) {
            success = false;
            break;
        }
        char* content = line_content(text);
        if (*content == '@') {
            success = read_annotation(sections, &current_section, content, content == text,
                line_offset, next_line_offset, line_number, error);
        } else if (*content && sections->implementation == MODULE_C_IMPLEMENTATION) {
            success = error_set(error, ERROR_INVALID, line_number,
                "@C must not contain an Ama code body; use the companion .c file");
        } else if (*content && current_section == SECTION_NONE) {
            success = error_set(error, ERROR_INVALID, line_number,
                "content before a section annotation; expected @<, @>, @Ama, or @C");
        }
        if (!success) break;
        line_offset = next_line_offset;
        line_number++;
    }
    free(text);
    if (!success) return false;
    if (ferror(source_file))
        return error_set(error, ERROR_IO, line_number, "cannot read source sections");
    end_section(sections, current_section, line_offset);
    return true;
}
