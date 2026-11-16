/**
 * @file
 * @brief Declare native identifier/record parsing independently of Ama word recognition.
 *
 * [PROTO-02](../../../spec/implementation/native-interface.md#proto-02)
 * Bindings come only from explicit marker records, never inferred C function names.
 */
#ifndef AMA_PARSER_ABI_INTERNAL_H
#define AMA_PARSER_ABI_INTERNAL_H
#include <stdint.h>
#include "abi.h"

/** Validate one nonempty retained native C identifier/UCN spelling, rejecting exact keyword text.
 * @param[out] error_offset Original byte offset of the first invalid scalar or initial offset for empty/keyword input.
 * @param[in] text Live bounded original source span containing one native identifier spelling.
 * Component contract: [PROTO-02](../../../spec/implementation/native-interface.md#proto-02). */
bool abi_identifier(SourceSpan text, size_t *error_offset);
/** Test exact supported C scalar ranges and initial/continuation restrictions; never use Ama XID tables as a substitute.
 * Component contract: [PROTO-02](../../../spec/implementation/native-interface.md#proto-02). */
bool abi_identifier_scalar(uint32_t scalar, bool initial);
/** Check flat ABI blank/trailing whitespace, allowing arbitrary comment indentation. Component contract: [PROTO-02](../../../spec/implementation/native-interface.md#proto-02). */
bool abi_line(AbiTable *, const LayoutLine *);
/** Parse one unindented marker/function record into its unique slot; duplicate failure retains the previous marker as related context.
 * Component contract: [PROTO-02](../../../spec/implementation/native-interface.md#proto-02). */
bool abi_record(AbiTable *table, const LayoutLine *line);
/** Publish one exact ABI parser failure through the common borrowed diagnostic boundary.
 * Component contract: [PROTO-02](../../../spec/implementation/native-interface.md#proto-02). */
bool abi_fail(AbiTable *table, SourceSpan span, const char *message, ParserExpectation expected);
#endif
