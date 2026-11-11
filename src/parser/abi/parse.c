#include "internal.h"

void abi_table_init(AbiTable *table, SourceAllocator allocator) {
    *table = (AbiTable){0};
    layout_init(&table->layout, allocator);
}

void abi_table_destroy(AbiTable *table) {
    layout_destroy(&table->layout);
    table->valid = false;
}

ParseStatus parser_abi_parse(AbiTable *table, const SourceText *source) {
    LayoutStatus read = layout_read_abi(&table->layout, source);
    if (read == LAYOUT_OK) {
        read = layout_validate(&table->layout, true);
    }
    if (read != LAYOUT_OK) {
        parser_error_from_layout(&table->error, &table->layout.diagnostic);
        return read == LAYOUT_OUT_OF_MEMORY ? PARSE_OUT_OF_MEMORY : PARSE_INVALID;
    }
    for (const LayoutLine *line = table->layout.first; line != NULL; line = line->next) {
        if (line->kind != LINE_BLANK && line->kind != LINE_COMMENT && !abi_record(table, line)) {
            return PARSE_INVALID;
        }
    }
    if (!table->bindings[ABI_DEFAULT].present || !table->bindings[ABI_DESTROY].present) {
        size_t end = source_length(source);
        abi_fail(table, (SourceSpan){source, end, end}, "Mandatory ABI lifecycle bindings are missing",
            !table->bindings[ABI_DEFAULT].present ? EXPECT_ABI_CONSTRUCTOR : EXPECT_ABI_DESTRUCTOR);
        return PARSE_INVALID;
    }
    table->valid = true;
    return PARSE_OK;
}
