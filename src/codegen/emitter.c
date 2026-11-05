#include "codegen/emitter.h"
#include "symbols/table.h"
#include <string.h>

void emitter_init(Emitter* emitter, FILE* output, SymbolTable* symbols) {
    *emitter = (Emitter){output, symbols};
}

void emit_break(Emitter* emitter) {
    fprintf(emitter->output, "    break;\n");
}

void emit_return(Emitter* emitter) {
    fprintf(emitter->output, "    return;\n");
}

void emit_branch_start(Emitter* emitter, uint32_t condition_symbol_id) {
    fprintf(emitter->output, "    if (symbol_%u) {\n", condition_symbol_id);
}

void emit_branch_else(Emitter* emitter) {
    fprintf(emitter->output, "    } else {\n");
}

void emit_branch_end(Emitter* emitter) {
    fprintf(emitter->output, "    }\n");
}

void emit_loop_start(Emitter* emitter, uint32_t condition_symbol_id) {
    fprintf(emitter->output, "    while (symbol_%u) {\n", condition_symbol_id);
}

void emit_loop_end(Emitter* emitter) {
    fprintf(emitter->output, "    }\n");
}

void emit_call_init(Emitter* emitter, const char* callee_path) {
    for (uint32_t index = 0; index < emitter->symbols->symbol_count; index++) {
        Symbol* symbol = &emitter->symbols->symbols[index];
        if (strcmp(symbol->module_path, callee_path) == 0 && symbol->kind != SYMBOL_MODULE_ALIAS)
            fprintf(emitter->output, "    symbol_%u = %d;\n", symbol->symbol_id, symbol->default_value);
    }
}

void emit_module_name(FILE* output, const char* path) {
    fputs("module_", output);
    for (const char* path_character = path; *path_character; path_character++)
        fputc(*path_character == '/' ? '_' : *path_character, output);
}

void emit_call_exec(Emitter* emitter, const char* callee_path) {
    fputs("    ", emitter->output);
    emit_module_name(emitter->output, callee_path);
    fputs("();\n", emitter->output);
}
