#include "codegen/emitter.h"
#include "symbols/table.h"
#include <string.h>

FILE* G_Out = NULL;

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

void emit_call_init(const char* callee_path) {
    for (uint32_t i = 0; i < G_Table.count; i++) {
        Symbol* s = &G_Table.symbols[i];
        if (strcmp(s->module_path, callee_path) == 0 && s->kind != SYM_ALIAS)
            fprintf(G_Out, "  v_%u = %d;\n", s->id, s->default_int);
    }
}

void emit_module_name(FILE* out, const char* path) {
    fputs("mod_", out);
    for (const char* p = path; *p; p++)
        fputc(*p == '/' ? '_' : *p, out);
}

void emit_call_exec(const char* callee_path) {
    fputs("  ", G_Out);
    emit_module_name(G_Out, callee_path);
    fputs("();\n", G_Out);
}
