#include "codegen/typed_native.h"

static void emit_state_type(FILE* output, const char* path) {
    emit_generated_name(output, NAME_STATE, path, NULL);
}

static void emit_signature(FILE* output, const PlannedType* type) {
    const char* path = type->source->module_path;
    if (type->frame.count && type->frame.declarations[0]->name.location.path)
        emit_line_directive(output, type->frame.declarations[0]->name.location.path,
            type->frame.declarations[0]->name.location.line);
    fputs("AmaStatus ", output);
    emit_generated_name(output, NAME_TYPED_NATIVE, path, NULL);
    fputc('(', output);
    emit_state_type(output, path);
    fputs("* state", output);
    for (int result = 0; result < 2; result++) {
        for (size_t index = 0; index < type->frame.count; index++) {
            const AstDeclaration* declaration = type->frame.declarations[index];
            if ((declaration->kind == AST_RESULT_DECLARATION) != (result != 0)) continue;
            fputs(result ? ", " : ", const ", output);
            emit_state_type(output, declaration->resolved_type);
            fputs("* ", output);
            emit_generated_name(output, result ? NAME_RESULT : NAME_PARAMETER, path, declaration->name.text);
        }
    }
    fputs(");\n\n", output);
    emit_line_directive(output, GENERATED_SOURCE_PATH, 1);
}

static void emit_bridge(FILE* output, const PlannedType* type) {
    const char* path = type->source->module_path;
    fputs("static inline AmaStatus ", output);
    emit_generated_name(output, NAME_NATIVE_BRIDGE, path, NULL);
    fputs("(void* state, void* frame) {\n    ", output);
    emit_generated_name(output, NAME_FRAME, path, NULL);
    fputs("* prepared = frame;\n    (void)prepared;\n    return ", output);
    emit_generated_name(output, NAME_TYPED_NATIVE, path, NULL);
    fputs("(state", output);
    for (int result = 0; result < 2; result++) {
        for (size_t index = 0; index < type->frame.count; index++) {
            const AstDeclaration* declaration = type->frame.declarations[index];
            if ((declaration->kind == AST_RESULT_DECLARATION) != (result != 0)) continue;
            fputs(", &prepared->", output);
            emit_generated_name(output, NAME_FIELD, path, declaration->name.text);
        }
    }
    fputs(");\n}\n\n", output);
}

static size_t emit_interface_fields(FILE* output, const PlannedType* type, bool results) {
    size_t count = 0;
    for (size_t index = 0; index < type->frame.count; index++)
        if ((type->frame.declarations[index]->kind == AST_RESULT_DECLARATION) == results) count++;
    if (!count) return 0;
    fprintf(output, "    static const AmaField %s[] = {\n", results ? "results" : "parameters");
    for (size_t index = 0; index < type->frame.count; index++) {
        const AstDeclaration* declaration = type->frame.declarations[index];
        if ((declaration->kind == AST_RESULT_DECLARATION) != results) continue;
        fputs("        {offsetof(", output);
        emit_generated_name(output, NAME_FRAME, type->source->module_path, NULL);
        fputs(", ", output);
        emit_generated_name(output, NAME_FIELD, type->source->module_path, declaration->name.text);
        fputs("), &", output);
        emit_generated_name(output, NAME_TYPE, declaration->resolved_type, NULL);
        fputs("},\n", output);
    }
    fputs("    };\n", output);
    return count;
}

void emit_typed_native_bindings(FILE* output, const PlannedType* type) {
    const char* path = type->source->module_path;
    emit_signature(output, type);
    emit_bridge(output, type);
    fputs("static inline const AmaCall* ", output);
    emit_generated_name(output, NAME_CALL_SCHEMA, path, NULL);
    fputs("(void) {\n", output);
    size_t parameters = emit_interface_fields(output, type, false);
    size_t results = emit_interface_fields(output, type, true);
    fputs("    static const AmaCall call = {\n        .state_type = &", output);
    emit_generated_name(output, NAME_TYPE, path, NULL);
    fputs(",\n        .frame_type = &", output);
    emit_generated_name(output, NAME_FRAME_TYPE, path, NULL);
    fprintf(output, ",\n        .parameters = %s,\n        .parameter_count = %zu,\n        .results = %s,\n        .result_count = %zu,\n        .execute = ",
        parameters ? "parameters" : "NULL", parameters, results ? "results" : "NULL", results);
    emit_generated_name(output, NAME_NATIVE_BRIDGE, path, NULL);
    fputs("\n    };\n    return &call;\n}\n\nstatic inline AmaStatus ", output);
    emit_generated_name(output, NAME_INVOKE, path, NULL);
    fputc('(', output);
    emit_state_type(output, path);
    fputs("* state, ", output);
    emit_generated_name(output, NAME_FRAME, path, NULL);
    fputs("* retained, const AmaCallInput* inputs, size_t input_count) {\n    return ama_call_execute(", output);
    emit_generated_name(output, NAME_CALL_SCHEMA, path, NULL);
    fputs("(),\n        (AmaValue){&", output);
    emit_generated_name(output, NAME_TYPE, path, NULL);
    fputs(", state}, (AmaValue){&", output);
    emit_generated_name(output, NAME_FRAME_TYPE, path, NULL);
    fputs(", retained}, inputs, input_count);\n}\n\n", output);
}
