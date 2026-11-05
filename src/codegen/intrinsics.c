#include "codegen/intrinsics.h"
#include "codegen/emitter.h"
#include "symbols/table.h"
#include <stdlib.h>
#include <string.h>

typedef enum { INTRINSIC_BINARY_OPERATION, INTRINSIC_DIVISION, INTRINSIC_LOGICAL_NOT, INTRINSIC_PRINT } IntrinsicOperationKind;

typedef struct {
    const char* name;
    const char* parameter_names[MAX_INTRINSIC_PARAMETERS];
    const char* result_names[MAX_INTRINSIC_RESULTS];
    int parameter_count;
    int result_count;
    IntrinsicOperationKind kind;
    const char* operator_text;
} IntrinsicDefinition;

/* Signatures and C operators stay together; no per-operation function wrappers. */
static const IntrinsicDefinition intrinsic_definitions[] = {
    {"sys/math/add", {"left","right"}, {"sum"},          2, 1, INTRINSIC_BINARY_OPERATION, "+"},
    {"sys/math/sub", {"left","right"}, {"diff"},         2, 1, INTRINSIC_BINARY_OPERATION, "-"},
    {"sys/math/mul", {"left","right"}, {"prod"},         2, 1, INTRINSIC_BINARY_OPERATION, "*"},
    {"sys/math/div", {"left","right"}, {"quot","error"}, 2, 2, INTRINSIC_DIVISION, "/"},
    {"sys/math/mod", {"left","right"}, {"rem","error"},  2, 2, INTRINSIC_DIVISION, "%"},
    {"sys/math/lt",  {"a","b"},        {"result"},       2, 1, INTRINSIC_BINARY_OPERATION, "<"},
    {"sys/math/le",  {"a","b"},        {"result"},       2, 1, INTRINSIC_BINARY_OPERATION, "<="},
    {"sys/math/gt",  {"a","b"},        {"result"},       2, 1, INTRINSIC_BINARY_OPERATION, ">"},
    {"sys/math/ge",  {"a","b"},        {"result"},       2, 1, INTRINSIC_BINARY_OPERATION, ">="},
    {"sys/math/eq",  {"a","b"},        {"result"},       2, 1, INTRINSIC_BINARY_OPERATION, "=="},
    {"sys/math/ne",  {"a","b"},        {"result"},       2, 1, INTRINSIC_BINARY_OPERATION, "!="},
    {"sys/math/and", {"a","b"},        {"result"},       2, 1, INTRINSIC_BINARY_OPERATION, "&&"},
    {"sys/math/or",  {"a","b"},        {"result"},       2, 1, INTRINSIC_BINARY_OPERATION, "||"},
    {"sys/math/not", {"a"},            {"result"},       1, 1, INTRINSIC_LOGICAL_NOT, NULL},
    {"sys/io/print", {"value"},        {"error"},        1, 1, INTRINSIC_PRINT, NULL},
};

static const int intrinsic_definition_count =
    (int)(sizeof(intrinsic_definitions) / sizeof(intrinsic_definitions[0]));

int is_intrinsic_module_path(const char* path) {
    for (int index = 0; index < intrinsic_definition_count; index++)
        if (strcmp(intrinsic_definitions[index].name, path) == 0)
            return 1;
    return 0;
}

static bool register_names(SymbolTable* symbols, const IntrinsicDefinition* intrinsic,
    const char* const* names, int name_count, SymbolKind kind,
    Error* error) {
    Token default_token = {.type = TOKEN_LITERAL, .lexeme = "0"};
    for (int index = 0; index < name_count; index++) {
        if (!names[index])
            return error_set(error, ERROR_INVALID, 0,
                "intrinsic '%s' has an incomplete signature", intrinsic->name);
        if (!add_declarative_symbol(symbols, intrinsic->name, names[index], kind, default_token, error))
            return false;
    }
    return true;
}

bool register_intrinsics(SymbolTable* symbols, Error* error) {
    for (int index = 0; index < intrinsic_definition_count; index++) {
        const IntrinsicDefinition* intrinsic = &intrinsic_definitions[index];
        if (!register_names(symbols, intrinsic, intrinsic->parameter_names, intrinsic->parameter_count, SYMBOL_PARAMETER, error) ||
            !register_names(symbols, intrinsic, intrinsic->result_names, intrinsic->result_count, SYMBOL_RESULT, error))
            return false;
    }
    return true;
}

static bool resolve_symbol_ids(SymbolTable* symbols, const IntrinsicDefinition* intrinsic,
    const char* const* names, int name_count, uint32_t* symbol_ids,
    Error* error) {
    for (int index = 0; index < name_count; index++) {
        Symbol* symbol = lookup_symbol(symbols, intrinsic->name, names[index]);
        if (!symbol)
            return error_set(error, ERROR_INVALID, 0,
                "intrinsic '%s' name '%s' not registered", intrinsic->name, names[index]);
        symbol_ids[index] = symbol->symbol_id;
    }
    return true;
}

/* Division and modulo share the explicit error-result convention. */
static void emit_division(FILE* output, const char* operator_text,
    const uint32_t* parameter_ids, const uint32_t* result_ids) {
    fprintf(output, "    size_t divisor = symbol_%u;\n", parameter_ids[1]);
    fprintf(output, "    if (divisor == 0) {\n");
    fprintf(output, "        symbol_%u = 0;\n", result_ids[0]);
    fprintf(output, "        symbol_%u = 1;\n", result_ids[1]);
    fprintf(output, "        return;\n    }\n");
    fprintf(output, "    symbol_%u = symbol_%u %s divisor;\n", result_ids[0], parameter_ids[0], operator_text);
    fprintf(output, "    symbol_%u = 0;\n", result_ids[1]);
}

static void emit_intrinsic_body(FILE* output, const IntrinsicDefinition* intrinsic,
    const uint32_t* parameter_ids, const uint32_t* result_ids) {
    switch (intrinsic->kind) {
    case INTRINSIC_BINARY_OPERATION:
        /* C comparisons and logical operators already return exactly 0 or 1. */
        fprintf(output, "    symbol_%u = (symbol_%u %s symbol_%u);\n", result_ids[0], parameter_ids[0], intrinsic->operator_text, parameter_ids[1]);
        break;
    case INTRINSIC_DIVISION:
        emit_division(output, intrinsic->operator_text, parameter_ids, result_ids);
        break;
    case INTRINSIC_LOGICAL_NOT:
        fprintf(output, "    symbol_%u = !symbol_%u;\n", result_ids[0], parameter_ids[0]);
        break;
    case INTRINSIC_PRINT:
        fprintf(output, "    printf(\"%%zu\\n\", symbol_%u);\n", parameter_ids[0]);
        fprintf(output, "    symbol_%u = 0;\n", result_ids[0]);
        break;
    }
}

bool emit_intrinsic_modules(FILE* output, SymbolTable* symbols, Error* error) {
    for (int index = 0; index < intrinsic_definition_count; index++) {
        const IntrinsicDefinition* intrinsic = &intrinsic_definitions[index];
        uint32_t parameter_ids[MAX_INTRINSIC_PARAMETERS];
        uint32_t result_ids[MAX_INTRINSIC_RESULTS];
        if (!resolve_symbol_ids(symbols, intrinsic, intrinsic->parameter_names, intrinsic->parameter_count, parameter_ids, error) ||
            !resolve_symbol_ids(symbols, intrinsic, intrinsic->result_names, intrinsic->result_count, result_ids, error))
            return false;
        fputs("void ", output);
        emit_module_name(output, intrinsic->name);
        fputs("(void) {\n", output);
        emit_intrinsic_body(output, intrinsic, parameter_ids, result_ids);
        fputs("}\n\n", output);
    }
    return true;
}
