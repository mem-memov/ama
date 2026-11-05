#ifndef AMA_CODEGEN_EMITTER_H
#define AMA_CODEGEN_EMITTER_H

#include <stdio.h>
#include <stdint.h>

extern FILE* G_Out;
/* Small output primitives can inline even when LTO is disabled. */
static inline void emit_assign_const(uint32_t dst, int val) {
    fprintf(G_Out, "  v_%u = %d;\n", dst, val);
}
static inline void emit_copy(uint32_t dst, uint32_t src) {
    fprintf(G_Out, "  v_%u = v_%u;\n", dst, src);
}
void emit_break(void);
void emit_return(void);
void emit_branch_start(uint32_t cond);
void emit_branch_else(void);
void emit_branch_end(void);
void emit_loop_start(uint32_t cond);
void emit_loop_end(void);
void emit_call_init(const char* callee_path);
void emit_call_exec(const char* callee_path);
void emit_module_name(FILE* out, const char* path);
void backend_emit_prologue(FILE* out);
void backend_emit_main(FILE* out, const char* entry);

#endif
