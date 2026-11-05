#include "ama.h"

/* The driver buffers generated bodies in G_Out until all symbols are known. */

FILE* G_Out = NULL;

void emit_assign_const(uint32_t dst, int val) {
    fprintf(G_Out, "  v_%u = %d;\n", dst, val);
}

void emit_assign_var(uint32_t dst, uint32_t src) {
    fprintf(G_Out, "  v_%u = v_%u;\n", dst, src);
}

/* ============================================================================
 * Break and return
 * ============================================================================
 *
 * `!` exits the nearest enclosing loop; C's `break` does exactly that.
 *
 * Bare `>` is an early return from the current function; C's `return` does
 * exactly that (all functions are void).
 */

void emit_break(void) {
    fprintf(G_Out, "  break;\n");
}

void emit_return(void) {
    fprintf(G_Out, "  return;\n");
}

void emit_branch_start(uint32_t cond) {
    fprintf(G_Out, "  if (v_%u) {\n", cond);
}

void emit_branch_else(void) {
    fprintf(G_Out, "  } else {\n");
}

void emit_branch_end(void) {
    fprintf(G_Out, "  }\n");
}

void emit_loop_start(uint32_t cond) {
    fprintf(G_Out, "  while (v_%u) {\n", cond);
}

void emit_loop_end(void) {
    fprintf(G_Out, "  }\n");
}

/* ============================================================================
 * Call: phase 1 — initialize callee's variables with their defaults
 * ============================================================================
 *
 * Before writing any parameters, every non-alias variable of the callee is
 * reset to its declared default. This guarantees a predictable starting state
 * for every call and matches the specification's "defaults → args → code →
 * results" model.
 */

void emit_call_init(const char* callee_path) {
    for (uint32_t i = 0; i < G_Table.count; i++) {
        Symbol* s = &G_Table.symbols[i];
        if (strcmp(s->module_path, callee_path) == 0 && s->kind != SYM_ALIAS)
            fprintf(G_Out, "  v_%u = %d;\n", s->id, s->default_int);
    }
}

void emit_call_param_val(uint32_t dst, uint32_t src) {
    fprintf(G_Out, "  v_%u = v_%u;\n", dst, src);
}

void emit_call_param_const(uint32_t dst, int val) {
    fprintf(G_Out, "  v_%u = %d;\n", dst, val);
}

void emit_call_result(uint32_t dst, uint32_t src) {
    fprintf(G_Out, "  v_%u = v_%u;\n", dst, src);
}

/* ============================================================================
 * Call: execute
 * ============================================================================
 *
 * Module "math/add" becomes a C function "mod_math_add". Slashes in the path
 * become underscores; dots would too, but paths are already normalized to
 * slashes before this point.
 */

void emit_call_exec(const char* callee_path) {
    fprintf(G_Out, "  mod_");
    for (const char* p = callee_path; *p; p++)
        fputc(*p == '/' ? '_' : *p, G_Out);
    fprintf(G_Out, "();\n");
}
