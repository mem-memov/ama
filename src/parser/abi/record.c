/**
 * @file
 * @brief Select exact fixed ABI markers and diagnose duplicate or malformed records.
 *
 * [PROTO-11](../../../spec/implementation/native-interface.md#proto-11)
 * Retain both marker sites on duplicate bindings; stricter global separators remain T022.
 */
#include "internal.h"

bool abi_fail(AbiTable *table, SourceSpan span, const char *message, ParserExpectation expected) {
    return parser_error_set(&table->error, span, message, expected);
}

/** Recognize one of the six fixed marker byte sequences and publish its consumed width; unknown markers return -1.
 */
static int operation(const unsigned char *bytes, size_t length, size_t *width) {
    static const unsigned char markers[] = {'+', '-', '|', '=', '?'};
    for (int index = 0; index < 5; index++) {
        if (length != 0 && bytes[0] == markers[index]) {
            *width = 1;
            return index;
        }
    }
    if (length >= 2 && bytes[0] == '[' && bytes[1] == ']') {
        *width = 2;
        return ABI_INDEX;
    }
    return -1;
}

/** Require retained separator plus one native identifier and no extra token. Publish borrowed spelling only for subsequent exact validation; no C declaration/signature inference.
 */
static bool binding_function(AbiTable *table, SourceSpan text, size_t cursor, SourceSpan *function) {
    const unsigned char *bytes = source_bytes(text.source);
    if (cursor == text.end) {
        return abi_fail(table, (SourceSpan){text.source, cursor, cursor},
            "An ABI record needs its native function identifier", EXPECT_C_IDENTIFIER);
    }
    if (bytes[cursor] != ' ') {
        return abi_fail(table, (SourceSpan){text.source, cursor, cursor},
            "An ABI symbol needs a space before its function", EXPECT_ABI_SPACE);
    }
    while (cursor < text.end && bytes[cursor] == ' ') {
        cursor++;
    }
    size_t start = cursor;
    while (cursor < text.end && bytes[cursor] != ' ') {
        cursor++;
    }
    *function = (SourceSpan){text.source, start, cursor};
    size_t error;
    if (!abi_identifier(*function, &error)) {
        return abi_fail(table, (SourceSpan){text.source, error, error < function->end ? error + 1 : error},
            "Invalid native C function identifier", EXPECT_C_IDENTIFIER);
    }
    if (cursor != text.end) {
        while (cursor < text.end && bytes[cursor] == ' ') {
            cursor++;
        }
        return abi_fail(table, (SourceSpan){text.source, cursor, text.end},
            "Extra tokens after an ABI function identifier", EXPECT_LINE_END);
    }
    return true;
}

bool abi_record(AbiTable *table, const LayoutLine *line) {
    SourceSpan text = line->code;
    if (line->indentation != 0) {
        return abi_fail(table, text, "ABI records start at column one", EXPECT_ABI_SYMBOL);
    }
    size_t width;
    int selected = operation(source_bytes(text.source) + text.start, text.end - text.start, &width);
    if (selected < 0) {
        return abi_fail(table, text, "Unknown ABI symbol", EXPECT_ABI_SYMBOL);
    }
    SourceSpan marker = {text.source, text.start, text.start + width};
    AbiBinding *binding = &table->bindings[selected];
    if (binding->present) {
        abi_fail(table, marker, "An ABI symbol can be bound only once", EXPECT_ABI_NEW_SYMBOL);
        table->related = (DiagnosticRelated){{binding->marker, NULL}, "previous binding", NULL};
        table->error.diagnostic.related = &table->related;
        table->error.diagnostic.related_count = 1;
        return false;
    }
    SourceSpan function;
    if (!binding_function(table, text, text.start + width, &function)) {
        return false;
    }
    *binding = (AbiBinding){true, marker, function};
    return true;
}
