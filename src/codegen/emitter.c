#include "codegen/emitter.h"
#include "symbols/table.h"
#include <string.h>

void emitter_init(Emitter* emitter, FILE* out, SymbolTable* symbols) {
    *emitter = (Emitter){out, symbols};
}

void emit_break(Emitter* emitter) {
    fprintf(emitter->out, "  break;\n");
}

void emit_return(Emitter* emitter) {
    fprintf(emitter->out, "  return;\n");
}

void emit_branch_start(Emitter* emitter, uint32_t cond) {
    fprintf(emitter->out, "  if (v_%u) {\n", cond);
}

void emit_branch_else(Emitter* emitter) {
    fprintf(emitter->out, "  } else {\n");
}

void emit_branch_end(Emitter* emitter) {
    fprintf(emitter->out, "  }\n");
}

void emit_loop_start(Emitter* emitter, uint32_t cond) {
    fprintf(emitter->out, "  while (v_%u) {\n", cond);
}

void emit_loop_end(Emitter* emitter) {
    fprintf(emitter->out, "  }\n");
}

void emit_call_init(Emitter* emitter, const char* callee_path) {
    for (uint32_t i = 0; i < emitter->symbols->count; i++) {
        Symbol* s = &emitter->symbols->symbols[i];
        if (strcmp(s->module_path, callee_path) == 0 && s->kind != SYM_ALIAS)
            fprintf(emitter->out, "  v_%u = %d;\n", s->id, s->default_int);
    }
}

void emit_module_name(FILE* out, const char* path) {
    fputs("mod_", out);
    for (const char* p = path; *p; p++)
        fputc(*p == '/' ? '_' : *p, out);
}

void emit_call_exec(Emitter* emitter, const char* callee_path) {
    fputs("  ", emitter->out);
    emit_module_name(emitter->out, callee_path);
    fputs("();\n", emitter->out);
}
