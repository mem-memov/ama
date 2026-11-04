#include "ama.h"

/* ============================================================================
 * Output stream
 * ============================================================================
 *
 * All emitters write to G_Out. The driver opens G_Out once (after pass 1 and
 * topological sort), emits prologue, intrinsics, module bodies in topological
 * order, and main(). Then it closes G_Out.
 */

FILE* G_Out = NULL;

/* ============================================================================
 * Assignment: direct
 * ============================================================================
 *
 * storage[dst] = <value>
 * where <value> is one of:
 *   - an integer literal
 *   - another variable's value (storage[src])
 *   - the value pointed to by another variable (storage[storage[ptr]])
 *   - the address of another variable (its offset, as a number)
 */

void emit_assign_const(uint32_t dst, int val) {
    fprintf(G_Out, "  storage[%u] = %d;\n", dst, val);
}

void emit_assign_var(uint32_t dst, uint32_t src) {
    fprintf(G_Out, "  storage[%u] = storage[%u];\n", dst, src);
}

void emit_assign_load_ind(uint32_t dst, uint32_t ptr) {
    fprintf(G_Out, "  storage[%u] = storage[storage[%u]];\n", dst, ptr);
}

void emit_assign_addr(uint32_t dst, uint32_t addr) {
    fprintf(G_Out, "  storage[%u] = %u;\n", dst, addr);
}

/* ============================================================================
 * Assignment: indirect (receiver is @ptr)
 * ============================================================================
 *
 * storage[storage[ptr]] = <value>
 * where <value> is one of the four forms above.
 */

void emit_store_indirect_const(uint32_t ptr, int val) {
    fprintf(G_Out, "  storage[storage[%u]] = %d;\n", ptr, val);
}

void emit_store_indirect_var(uint32_t ptr, uint32_t src) {
    fprintf(G_Out, "  storage[storage[%u]] = storage[%u];\n", ptr, src);
}

void emit_store_indirect_load_ind(uint32_t ptr, uint32_t src_ptr) {
    fprintf(G_Out, "  storage[storage[%u]] = storage[storage[%u]];\n",
            ptr, src_ptr);
}

void emit_store_indirect_addr(uint32_t ptr, uint32_t addr) {
    fprintf(G_Out, "  storage[storage[%u]] = %u;\n", ptr, addr);
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

/* ============================================================================
 * Branching
 * ============================================================================
 *
 * Condition is a variable name; its storage slot holds a value. Zero means
 * false; anything non-zero means true.
 *
 *   if (storage[cond]) {
 *     <true body>
 *   } else {
 *     <false body>
 *   }
 */

void emit_branch_start(uint32_t cond) {
    fprintf(G_Out, "  if (storage[%u]) {\n", cond);
}

void emit_branch_else(void) {
    fprintf(G_Out, "  } else {\n");
}

void emit_branch_end(void) {
    fprintf(G_Out, "  }\n");
}

/* ============================================================================
 * Loop
 * ============================================================================
 *
 * The { } construct is a while-loop. The condition is a variable name; it is
 * re-read on each iteration. `!` inside the body exits the loop via break.
 *
 *   while (storage[cond]) {
 *     <body>
 *   }
 */

void emit_loop_start(uint32_t cond) {
    fprintf(G_Out, "  while (storage[%u]) {\n", cond);
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
            fprintf(G_Out, "  storage[%u] = %d;\n", s->offset, s->default_int);
    }
}

/* ============================================================================
 * Call: phase 2 — write parameters
 * ============================================================================
 *
 * Each parameter of the callee is stored in the callee's slot:
 *   storage[callee_param_offset] = <value>
 * where <value> is a variable, a constant, or an address.
 */

void emit_call_param_val(uint32_t dst, uint32_t src) {
    fprintf(G_Out, "  storage[%u] = storage[%u];\n", dst, src);
}

void emit_call_param_const(uint32_t dst, int val) {
    fprintf(G_Out, "  storage[%u] = %d;\n", dst, val);
}

void emit_call_param_addr(uint32_t dst, uint32_t addr) {
    fprintf(G_Out, "  storage[%u] = %u;\n", dst, addr);
}

/* ============================================================================
 * Call: phase 3 — copy results back
 * ============================================================================
 *
 * After the call, the caller copies each declared return slot of the callee
 * into its own variable:
 *   storage[caller_dst] = storage[callee_result_offset]
 */

void emit_call_result(uint32_t dst, uint32_t src) {
    fprintf(G_Out, "  storage[%u] = storage[%u];\n", dst, src);
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