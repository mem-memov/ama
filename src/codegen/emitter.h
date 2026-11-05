#ifndef AMA_CODEGEN_EMITTER_H
#define AMA_CODEGEN_EMITTER_H

#include <stdio.h>
#include <stdint.h>
#include "symbols/table.h"
#include "graph/graph.h"

typedef struct {
    FILE* output;
    SymbolTable* symbols;
    const char* module_path; /* Module whose body is being emitted; NULL outside a body. */
} Emitter;

void emitter_init(Emitter* emitter, FILE* output, SymbolTable* symbols);
/* Variable access selects the current state or a callee's default instance. */
void emit_symbol_access(Emitter* emitter, uint32_t symbol_id);
void emit_constant_assignment(Emitter* emitter, uint32_t destination_symbol_id, int value);
void emit_copy(Emitter* emitter, uint32_t destination_symbol_id, uint32_t source_symbol_id);
void emit_state_type(FILE* output, const char* path);
void emit_default_instance(FILE* output, const char* path);
void emit_module_start(Emitter* emitter, const char* path);
void emit_module_end(Emitter* emitter);
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
void emit_program_prologue(FILE* output, const SymbolTable* symbols, const Graph* graph);
void emit_program_entry_point(Emitter* emitter, const char* entry);

#endif
