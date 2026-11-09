#include "codegen/emitter.h"
#include "symbols/table.h"
#include <string.h>

void emitter_init(Emitter* emitter, FILE* output, SymbolTable* symbols) {
    *emitter = (Emitter){.output = output, .symbols = symbols};
}

void emit_source_location(Emitter* emitter, SourceLocation location) {
    if (emitter->source_mapping_enabled && location.path && location.line)
        emit_line_directive(emitter->output, location.path, location.line);
}

void emit_generated_location(Emitter* emitter) {
    if (emitter->source_mapping_enabled)
        emit_line_directive(emitter->output, GENERATED_SOURCE_PATH, 1);
}

void emit_state_type(FILE* output, const char* path) {
    emit_generated_name(output, NAME_STATE, path, NULL);
}

void emit_default_instance(FILE* output, const char* path) {
    emit_generated_name(output, NAME_INSTANCE, path, NULL);
}

void emit_symbol_access(Emitter* emitter, uint32_t symbol_id) {
    const Symbol* symbol = &emitter->symbols->symbols[symbol_id];
    if (emitter->module_path && strcmp(emitter->module_path, symbol->module_path) == 0) {
        fputs("state->", emitter->output);
    } else {
        emit_default_instance(emitter->output, symbol->module_path);
        fputc('.', emitter->output);
    }
    emit_generated_name(emitter->output, NAME_FIELD, symbol->module_path, symbol->name);
}

void emit_constant_assignment(Emitter* emitter, uint32_t destination_symbol_id, int value) {
    fputs("    ", emitter->output);
    emit_symbol_access(emitter, destination_symbol_id);
    fprintf(emitter->output, " = %d;\n", value);
}

void emit_copy(Emitter* emitter, uint32_t destination_symbol_id, uint32_t source_symbol_id) {
    fputs("    ", emitter->output);
    emit_symbol_access(emitter, destination_symbol_id);
    fputs(" = ", emitter->output);
    emit_symbol_access(emitter, source_symbol_id);
    fputs(";\n", emitter->output);
}

void emit_module_start(Emitter* emitter, const char* path) {
    emit_generated_location(emitter);
    emitter->module_path = path;
    fputs("void ", emitter->output);
    emit_module_name(emitter->output, path);
    fputc('(', emitter->output);
    emit_state_type(emitter->output, path);
    fputs("* state) {\n    (void)state;\n", emitter->output);
}

void emit_module_end(Emitter* emitter) {
    emit_generated_location(emitter);
    fputs("}\n\n", emitter->output);
    emitter->module_path = NULL;
}

void emit_break(Emitter* emitter) {
    fprintf(emitter->output, "    break;\n");
}

void emit_return(Emitter* emitter) {
    fprintf(emitter->output, "    return;\n");
}

void emit_branch_start(Emitter* emitter, uint32_t condition_symbol_id) {
    fputs("    if (", emitter->output);
    emit_symbol_access(emitter, condition_symbol_id);
    fputs(") {\n", emitter->output);
}

void emit_branch_else(Emitter* emitter) {
    fprintf(emitter->output, "    } else {\n");
}

void emit_branch_end(Emitter* emitter) {
    fprintf(emitter->output, "    }\n");
}

void emit_loop_start(Emitter* emitter, uint32_t condition_symbol_id) {
    fputs("    while (", emitter->output);
    emit_symbol_access(emitter, condition_symbol_id);
    fputs(") {\n", emitter->output);
}

void emit_loop_end(Emitter* emitter) {
    fprintf(emitter->output, "    }\n");
}

void emit_call_init(Emitter* emitter, const char* callee_path) {
    for (uint32_t index = 0; index < emitter->symbols->symbol_count; index++) {
        Symbol* symbol = &emitter->symbols->symbols[index];
        if (strcmp(symbol->module_path, callee_path) == 0 &&
            (symbol->kind == SYMBOL_PARAMETER || symbol->kind == SYMBOL_RESULT))
            emit_constant_assignment(emitter, symbol->symbol_id, symbol->default_value);
    }
}

void emit_module_name(FILE* output, const char* path) {
    emit_generated_name(output, NAME_CALL, path, NULL);
}

void emit_call_exec(Emitter* emitter, const char* callee_path) {
    fputs("    ", emitter->output);
    emit_module_name(emitter->output, callee_path);
    fputs("(&", emitter->output);
    emit_default_instance(emitter->output, callee_path);
    fputs(");\n", emitter->output);
}
