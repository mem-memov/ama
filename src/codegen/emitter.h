#ifndef AMA_CODEGEN_EMITTER_H
#define AMA_CODEGEN_EMITTER_H

#include <stdio.h>
#include <stdint.h>
#include "symbols/table.h"

typedef struct {
    FILE* out;
    SymbolTable* symbols;
} Emitter;

void emitter_init(Emitter* emitter, FILE* out, SymbolTable* symbols);
/* Small output primitives can inline even when LTO is disabled. */
static inline void emit_assign_const(Emitter* emitter, uint32_t dst, int val) {
    fprintf(emitter->out, "  v_%u = %d;\n", dst, val);
}
static inline void emit_copy(Emitter* emitter, uint32_t dst, uint32_t src) {
    fprintf(emitter->out, "  v_%u = v_%u;\n", dst, src);
}
void emit_break(Emitter* emitter);
void emit_return(Emitter* emitter);
void emit_branch_start(Emitter* emitter, uint32_t cond);
void emit_branch_else(Emitter* emitter);
void emit_branch_end(Emitter* emitter);
void emit_loop_start(Emitter* emitter, uint32_t cond);
void emit_loop_end(Emitter* emitter);
void emit_call_init(Emitter* emitter, const char* callee_path);
void emit_call_exec(Emitter* emitter, const char* callee_path);
void emit_module_name(FILE* out, const char* path);
void backend_emit_prologue(FILE* out, const SymbolTable* symbols);
void backend_emit_main(Emitter* emitter, const char* entry);

#endif
