/**
 * @file
 * @brief Check flat ABI physical whitespace independently of Ama literal tokenization.
 * [PROTO-11](../../../spec/implementation/native-interface.md#proto-11) uses
 * one C identifier per record; [SYN-06](../../../spec/language/source-and-names.md#syn-06)
 * forbids spaces-only lines and trailing code spaces without a comment.
 */
#include "internal.h"
bool abi_line(AbiTable *table, const LayoutLine *line) {
    if (line->kind == LINE_BLANK) {
        return line->physical.start == line->physical.end
            || abi_fail(table, line->physical, "An ABI blank line must contain no spaces", EXPECT_LINE_END);
    }
    if (line->kind == LINE_COMMENT) { return true; }
    const unsigned char *bytes = source_bytes(line->physical.source);
    size_t cursor = line->code.end;
    while (cursor < line->physical.end && bytes[cursor] == ' ') { cursor++; }
    if (cursor == line->physical.end && cursor != line->code.end) {
        return abi_fail(table, (SourceSpan){line->physical.source, line->code.end, cursor},
            "An ABI record cannot end in spaces without a comment", EXPECT_LINE_END);
    }
    return true;
}
