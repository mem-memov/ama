#ifndef AMA_PARSER_NATIVE_ABI_H
#define AMA_PARSER_NATIVE_ABI_H

#include "lexer/layout/layout.h"
#include "parser/status.h"

typedef enum {
    ABI_DEFAULT, ABI_DESTROY, ABI_LITERAL, ABI_CLONE, ABI_CONDITION, ABI_INDEX, ABI_BINDING_COUNT
} AbiOperation;
typedef struct {
    bool present;
    SourceSpan marker;
    SourceSpan function;
} AbiBinding;
typedef struct {
    SourceLayout layout;
    AbiBinding bindings[ABI_BINDING_COUNT];
    Diagnostic diagnostic;
    DiagnosticRelated related;
    bool valid;
} AbiTable;

/* Parse once after init. Function spellings remain borrowed source spans.
 * Signature and toolchain-specific function usability are verified by typed C
 * uses later. No inference from function names or source-module parsing. */
void abi_table_init(AbiTable *table, SourceAllocator allocator);
void abi_table_destroy(AbiTable *table);
ParseStatus parser_abi_parse(AbiTable *table, const SourceText *source);

#endif
