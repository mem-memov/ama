/**
 * @file
 * @brief Own six explicit native-protocol binding slots with original source spans.
 *
 * [PROTO-11](../../../spec/implementation/native-interface.md#proto-11)
 * Lexical parsing preserves native spellings; typed C compilation verifies actual callable signatures later.
 */
#ifndef AMA_PARSER_NATIVE_ABI_H
#define AMA_PARSER_NATIVE_ABI_H

#include "lexer/layout/layout.h"
#include "parser/status.h"
#include "parser/error/error.h"

/** The six explicitly bound native protocols; slots are selected by marker, not inferred C naming. */
typedef enum {
    ABI_DEFAULT, ABI_DESTROY, ABI_LITERAL, ABI_CLONE, ABI_CONDITION, ABI_INDEX, ABI_BINDING_COUNT
} AbiOperation;
/** One explicit marker/function spelling pair borrowing original ABI source bytes. */
typedef struct {
    bool present;
    SourceSpan marker;
    SourceSpan function;
} AbiBinding;
/** Owner of ABI layout plus borrowed slot/duplicate diagnostic spans; consume only after valid=true. */
typedef struct {
    SourceLayout layout;
    AbiBinding bindings[ABI_BINDING_COUNT];
    ParseError error;
    DiagnosticRelated related;
    bool valid;
} AbiTable;

/* Parse once after init. Function spellings remain borrowed source spans.
 * Signature and toolchain-specific function usability are verified by typed C
 * uses later. No inference from function names or source-module parsing. */
/** Initialize empty native binding slots and owned physical layout with borrowed allocator authority.
 */
void abi_table_init(AbiTable *table, SourceAllocator allocator);
/** Release complete/failed layout and invalidate the table. Native spelling spans remain borrowed from the external source owner.
 */
void abi_table_destroy(AbiTable *table);
/** Parse once, requiring explicit default/destruction slots and unique supported markers.
 * @return Success only with valid=true; lexical failure retains exact sites, duplicates retain both sites. C signatures/usability are checked later by typed compilation.
 */
ParseStatus parser_abi_parse(AbiTable *table, const SourceText *source);

#endif
