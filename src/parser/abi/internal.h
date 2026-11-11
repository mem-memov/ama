#ifndef AMA_PARSER_ABI_INTERNAL_H
#define AMA_PARSER_ABI_INTERNAL_H
#include <stdint.h>
#include "abi.h"

bool abi_identifier(SourceSpan text, size_t *error_offset);
bool abi_identifier_scalar(uint32_t scalar, bool initial);
bool abi_record(AbiTable *table, const LayoutLine *line);
bool abi_fail(AbiTable *table, SourceSpan span, const char *message, ParserExpectation expected);
#endif
