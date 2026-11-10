#include "codegen/type_plan.h"
#include "codegen/typed_native.h"
#include <inttypes.h>

static void emit_c_string(FILE* output, const char* text) {
    fputc('"', output);
    for (const unsigned char* byte = (const unsigned char*)text; *byte; byte++) {
        if (*byte == '"' || *byte == '\\') fprintf(output, "\\%c", *byte);
        else if (*byte < 32 || *byte == 127 || *byte == '?') fprintf(output, "\\%03o", *byte);
        else fputc(*byte, output);
    }
    fputc('"', output);
}

static void emit_field_access(FILE* output, const char* module_path, const AstDeclaration* declaration) {
    fputs("state->", output);
    emit_generated_name(output, NAME_FIELD, module_path, declaration->name.text);
}

static void emit_structure(FILE* output, const PlannedType* type, bool frame) {
    const char* path = type->source->module_path;
    const TypeFields* fields = frame ? &type->frame : &type->state;
    char name[GENERATED_NAME_CAPACITY];
    generated_name(name, frame ? NAME_FRAME : NAME_STATE, path, NULL);
    emit_source_label(output, path);
    fprintf(output, "typedef struct %s {\n", name);
    if (!fields->count) fputs("    unsigned char ama_empty;\n", output);
    for (size_t index = 0; index < fields->count; index++) {
        const AstDeclaration* declaration = fields->declarations[index];
        if (declaration->name.location.path)
            emit_line_directive(output, declaration->name.location.path, declaration->name.location.line);
        fputs("    ", output);
        emit_generated_name(output, NAME_STATE, declaration->resolved_type, NULL);
        fputc(' ', output);
        emit_generated_name(output, NAME_FIELD, path, declaration->name.text);
        fputs(";\n", output);
        emit_line_directive(output, GENERATED_SOURCE_PATH, 1);
    }
    fprintf(output, "} %s;\n\n", name);
}

static void emit_fields(FILE* output, const PlannedType* type, bool frame) {
    const char* path = type->source->module_path;
    const TypeFields* fields = frame ? &type->frame : &type->state;
    char structure[GENERATED_NAME_CAPACITY], array[GENERATED_NAME_CAPACITY], record[GENERATED_NAME_CAPACITY];
    generated_name(structure, frame ? NAME_FRAME : NAME_STATE, path, NULL);
    generated_name(array, frame ? NAME_FRAME_FIELDS : NAME_FIELDS, path, NULL);
    generated_name(record, frame ? NAME_FRAME_RECORD : NAME_RECORD, path, NULL);
    if (fields->count) {
        fprintf(output, "static const AmaField %s[] = {\n", array);
        for (size_t index = 0; index < fields->count; index++) {
            const AstDeclaration* declaration = fields->declarations[index];
            fprintf(output, "    {offsetof(%s, ", structure);
            emit_generated_name(output, NAME_FIELD, path, declaration->name.text);
            fputs("), &", output);
            emit_generated_name(output, NAME_TYPE, declaration->resolved_type, NULL);
            fputs("},\n", output);
        }
        fputs("};\n", output);
    }
    fprintf(output, "static const AmaRecord %s = {%s, %zu};\n\n", record,
        fields->count ? array : "NULL", fields->count);
}

static void emit_literal(FILE* output, const AstValue* value) {
    if (!value->literal_length) {
        fputs("NULL, 0", output);
        return;
    }
    fputs("(const unsigned char[]){", output);
    for (size_t index = 0; index < value->literal_length; index++) {
        if (index) fputs(", ", output);
        fprintf(output, "0x%02x", (unsigned char)value->literal_bytes[index]);
    }
    fprintf(output, "}, %zu", value->literal_length);
}

static void emit_field_initializer(FILE* output, const char* path, const AstDeclaration* declaration) {
    const AstInitializer* initializer = declaration->initializers;
    const char* operation = !initializer ? "initialize" :
        initializer->value.kind == AST_LINE_LITERAL_VALUE ? "initialize_literal" : "initialize_copy";
    SourceLocation location = initializer ? initializer->value.location : declaration->name.location;
    if (location.path) emit_line_directive(output, location.path, location.line);
    fprintf(output, "    status = ama_value_%s((AmaValue){&", operation);
    emit_generated_name(output, NAME_TYPE, declaration->resolved_type, NULL);
    fputs(", &", output);
    emit_field_access(output, path, declaration);
    fputc('}', output);
    if (initializer) {
        fputs(", ", output);
        if (initializer->value.kind == AST_LINE_LITERAL_VALUE) emit_literal(output, &initializer->value);
        else {
            fputs("(AmaValue){&", output);
            emit_generated_name(output, NAME_TYPE, initializer->source->resolved_type, NULL);
            fputs(", &", output);
            emit_field_access(output, path, initializer->source);
            fputc('}', output);
        }
    }
    fputs(");\n", output);
    emit_line_directive(output, GENERATED_SOURCE_PATH, 1);
    fputs("    if (status != AMA_STATUS_OK) goto failed;\n    initialized++;\n", output);
}

static void emit_initializer(FILE* output, const PlannedType* type, bool frame) {
    const char* path = type->source->module_path;
    const TypeFields* fields = frame ? &type->frame : &type->state;
    if (!fields->count) return;
    char name[GENERATED_NAME_CAPACITY], structure[GENERATED_NAME_CAPACITY];
    generated_name(name, frame ? NAME_FRAME_INITIALIZE : NAME_INITIALIZE, path, NULL);
    generated_name(structure, frame ? NAME_FRAME : NAME_STATE, path, NULL);
    fprintf(output, "static AmaStatus %s(const AmaType* type, void* destination) {\n", name);
    fprintf(output, "    %s* state = destination;\n", structure);
    fputs("    size_t initialized = 0;\n    AmaStatus status;\n    (void)type;\n", output);
    for (size_t index = 0; index < fields->count; index++)
        emit_field_initializer(output, path, fields->declarations[index]);
    fputs("    return AMA_STATUS_OK;\nfailed:\n    while (initialized) {\n        const AmaField* field = &", output);
    emit_generated_name(output, frame ? NAME_FRAME_FIELDS : NAME_FIELDS, path, NULL);
    fputs("[--initialized];\n        ama_value_destroy((AmaValue){field->type,\n"
        "            (unsigned char*)destination + field->offset});\n    }\n    return status;\n}\n\n", output);
}

static void emit_descriptor(FILE* output, const PlannedType* type, bool frame) {
    const char* path = type->source->module_path;
    const TypeFields* fields = frame ? &type->frame : &type->state;
    char structure[GENERATED_NAME_CAPACITY];
    generated_name(structure, frame ? NAME_FRAME : NAME_STATE, path, NULL);
    fputs("const AmaType ", output);
    emit_generated_name(output, frame ? NAME_FRAME_TYPE : NAME_TYPE, path, NULL);
    fprintf(output, " = {\n    .abi_version = AMA_VALUE_ABI_VERSION,\n    .identity = UINT64_C(0x%016" PRIx64 "),\n",
        source_identity_hash(path, frame ? "$frame" : NULL));
    fputs("    .module_path = ", output);
    emit_c_string(output, path);
    fprintf(output, ",\n    .size = sizeof(%s),\n    .alignment = _Alignof(%s),\n    .context = &", structure, structure);
    emit_generated_name(output, frame ? NAME_FRAME_RECORD : NAME_RECORD, path, NULL);
    fputs(",\n    .initialize = ", output);
    if (fields->count) emit_generated_name(output, frame ? NAME_FRAME_INITIALIZE : NAME_INITIALIZE, path, NULL);
    else fputs("ama_record_initialize", output);
    fputs(",\n    .initialize_copy = ama_record_initialize_copy,\n    .move = ama_record_move,\n"
        "    .destroy = ama_record_destroy\n};\n\n", output);
}

bool emit_type_layouts(FILE* header, FILE* implementation, const char* header_name,
    const TypeModule* modules, size_t module_count, Error* error) {
    if (!header || !implementation || header == implementation || !valid_type_header_name(header_name))
        return error_set(error, ERROR_INVALID, 0, "type generation requires distinct streams and a valid header name");
    TypePlan plan = {0};
    if (!build_type_plan(&plan, modules, module_count, error)) {
        destroy_type_plan(&plan);
        return false;
    }
    char guard[GENERATED_NAME_CAPACITY];
    generated_name(guard, NAME_HEADER, header_name, "$types");
    fprintf(header, "/* Generated by Ama; do not edit. */\n#ifndef %s\n#define %s\n", guard, guard);
    fputs("#include \"runtime/call.h\"\n\n", header);
    for (size_t index = 0; index < plan.count; index++)
        if (plan.types[index].source->native_header)
            fprintf(header, "#include \"%s\"\n", plan.types[index].source->native_header);
    for (size_t position = 0; position < plan.ordered_count; position++) {
        const PlannedType* type = &plan.types[plan.order[position]];
        if (!type->source->native_header) emit_structure(header, type, false);
    }
    for (size_t index = 0; index < plan.count; index++) {
        const PlannedType* type = &plan.types[index];
        if (type->source->native_header && !type->source->module) continue;
        emit_structure(header, type, true);
        if (!type->source->native_header) {
            fputs("extern const AmaType ", header);
            emit_generated_name(header, NAME_TYPE, type->source->module_path, NULL);
            fputs(";\n", header);
        }
        fputs("extern const AmaType ", header);
        emit_generated_name(header, NAME_FRAME_TYPE, type->source->module_path, NULL);
        fputs(";\n\n", header);
    }
    for (size_t index = 0; index < plan.count; index++)
        if (plan.types[index].source->native_header && plan.types[index].source->module)
            emit_typed_native_bindings(header, &plan.types[index]);
    fputs("#endif\n", header);
    fprintf(implementation, "/* Generated by Ama; do not edit. */\n#include \"%s\"\n\n", header_name);
    for (size_t index = 0; index < plan.count; index++) {
        const PlannedType* type = &plan.types[index];
        if (type->source->native_header && !type->source->module) continue;
        for (int frame = type->source->native_header ? 1 : 0; frame < 2; frame++) {
            emit_fields(implementation, type, frame != 0);
            emit_initializer(implementation, type, frame != 0);
            emit_descriptor(implementation, type, frame != 0);
        }
    }
    destroy_type_plan(&plan);
    if (ferror(header) || ferror(implementation))
        return error_set(error, ERROR_IO, 0, "cannot write generated type layouts");
    return true;
}
