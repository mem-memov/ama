#ifndef AMA_CODEGEN_EMITTER_H
#define AMA_CODEGEN_EMITTER_H

#include <stdio.h>
#include <stdint.h>
#include "symbols/table.h"

typedef struct {
    FILE* output;
    SymbolTable* symbols;
} Emitter;

void emitter_init(Emitter* emitter, FILE* output, SymbolTable* symbols);
/* Small output primitives can inline even when LTO is disabled. */
static inline void emit_constant_assignment(Emitter* emitter, uint32_t destination_symbol_id, int value) {
    fprintf(emitter->output, "    symbol_%u = %d;\n", destination_symbol_id, value);
}
static inline void emit_copy(Emitter* emitter, uint32_t destination_symbol_id, uint32_t source_symbol_id) {
    fprintf(emitter->output, "    symbol_%u = symbol_%u;\n", destination_symbol_id, source_symbol_id);
}
void emit_break(Emitter* emitter);
void emit_return(Emitter* emitter);
void emit_branch_start(Emitter* emitter, uint32_t condition_symbol_id);
void emit_branch_else(Emitter* emitter);
void emit_branch_end(Emitter* emitter);
void emit_loop_start(Emitter* emitter, uint32_t condition_symbol_id);
void emit_loop_end(Emitter* emitter);
void emit_call_init(Emitter* emitter, const char* callee_path);
void emit_call_exec(Emitter* emitter, const char* callee_path);
void emit_module_name(FILE* output, const char* path);
void emit_program_prologue(FILE* output, const SymbolTable* symbols);
void emit_program_entry_point(Emitter* emitter, const char* entry);

#endif
