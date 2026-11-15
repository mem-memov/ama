/**
 * @file
 * @brief Emit forward declarations, native representations and complete typed borrowed views.
 *
 * [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06)
 * All required types precede permitted field access; source locations retain original Ama names without runtime/body inclusion.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) Semantic spellings cover inferred fields alongside explicit declarations.
 * [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04) matches
 * representation-header error providers to their body contexts. [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03)
 * requires the authored standard size-helper declaration before a typed alias can bind it.
 */
#include "native/scaffold/internal.h"
#include "source/mapping/directive.h"

/** Declare all nominal view names and native State tags before dependency field use, permitting legal reference cycles.
 */
static bool forward(FILE *output, const ScaffoldType *type) {
    for (size_t role = 0; role < SCAFFOLD_ROLE_COUNT; role++) {
        const char *name = type->names[role]->internal_name;
        if (fprintf(output, "typedef struct %s %s;\n", name, name) < 0) {
            return false;
        }
    }
    return type->plan->type->module->syntax.sections.kind != MODULE_NATIVE
        || fprintf(output, "typedef struct %s_State %s_State;\n",
            type->native_prefix->internal_name, type->native_prefix->internal_name) >= 0;
}

/** Map one checked storage form and read/write capability to the exact generated view family.
 */
static ScaffoldTypeRole field_role(StorageForm form, bool write) {
    return form == STORAGE_REFERENCE ? (write ? SCAFFOLD_REFERENCE_WRITE : SCAFFOLD_REFERENCE_READ)
        : form == STORAGE_OWNING_LIST ? (write ? SCAFFOLD_LIST_WRITE : SCAFFOLD_LIST_READ)
        : form == STORAGE_REFERENCE_LIST ? (write ? SCAFFOLD_REFERENCE_LIST_WRITE : SCAFFOLD_REFERENCE_LIST_READ)
        : write ? SCAFFOLD_WRITE : SCAFFOLD_READ;
}

/** Emit only the selected whole/interface role with original source/name comments and exact qualified field identity; empty C structs receive a reserved byte.
 */
static bool fields(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type,
    bool write, bool interface, SymbolRole role) {
    size_t count = 0;
    for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
        if (interface && member->field->symbol->role != role) {
            continue;
        }
        const ScaffoldType *child = native_scaffold_type(scaffold, member->field->symbol->type);
        const char *name = child->names[field_role(member->field->form, write)]->internal_name;
        SourceSpan source_name = member->field->symbol->spelling;
        if (source_write_line_directive(output, source_name.source, source_name.start) != SOURCE_OK
            || fprintf(output, "    /* %zu: ", interface ? member->field->symbol->role_ordinal : member->field->symbol->ordinal) < 0
            || fwrite(source_bytes(source_name.source) + source_name.start, 1, source_name.end - source_name.start, output)
                != source_name.end - source_name.start
            || fprintf(output, " */\n    %s%s *%s;\n", write ? "" : "const ", name, member->identity->internal_name) < 0) {
            return false;
        }
        count++;
    }
    return count != 0 || fprintf(output, "    unsigned char reserved;\n") >= 0;
}

/** Emit opaque reference/list storage views plus scalar read/write views, retaining payload constness and typed field access.
 */
static bool views(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type) {
    const ScaffoldTypeRole reads[] = {SCAFFOLD_REFERENCE_READ, SCAFFOLD_LIST_READ, SCAFFOLD_REFERENCE_LIST_READ};
    for (size_t index = 0; index < 3; index++) {
        size_t read = reads[index];
        const char *storage = type->names[read - 1]->internal_name;
        if (read == SCAFFOLD_REFERENCE_READ) {
            if (fprintf(output, "struct %s { const %s *storage;\n"
                    "    AmaStatus (*with_read)(const %s *, AmaStatus (*)(const %s *, void *), void *); };\n"
                    "struct %s { %s *storage; };\n", type->names[read]->internal_name, storage,
                    storage, type->names[SCAFFOLD_READ]->internal_name,
                    type->names[read + 1]->internal_name, storage) < 0) { return false; }
        } else if (fprintf(output, "struct %s { const %s *storage; };\nstruct %s { %s *storage; };\n",
                type->names[read]->internal_name, storage, type->names[read + 1]->internal_name, storage) < 0) {
            return false;
        }
    }
    for (size_t role = SCAFFOLD_READ; role <= SCAFFOLD_WRITE; role++) {
        bool write = role == SCAFFOLD_WRITE;
        if (fprintf(output, "struct %s {\n", type->names[role]->internal_name) < 0) {
            return false;
        }
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE
            && fprintf(output, "    %s%s_State *payload;\n", write ? "" : "const ", type->native_prefix->internal_name) < 0) {
            return false;
        }
        if (!fields(scaffold, output, type, write, false, SYMBOL_STATE) || fputs("};\n", output) == EOF) {
            return false;
        }
    }
    return true;
}

bool scaffold_emit_types(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    if (fputs("#ifndef AMA_GENERATED_NATIVE_TYPES_H\n#define AMA_GENERATED_NATIVE_TYPES_H\n"
        "#include <stdbool.h>\n#include <stddef.h>\n#include <stdint.h>\n"
        "#include \"native/abi/bytes.h\"\n#include \"native/abi/services.h\"\n"
        "#include \"native/toolchain/gcc.h\"\n", output) == EOF) {
        return false;
    }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (!forward(output, type)) {
            return false;
        }
    }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE
            && (!native_namespace_begin(output, type->native_prefix)
                || !native_namespace_status(output, type->error_provider)
                || !scaffold_include(output, type->header_path) || !scaffold_emit_bridge(output, type)
                || !native_namespace_end(output))) {
            return false;
        }
    }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (!views(scaffold, output, type)) {
            return false;
        }
        for (SymbolRole role = SYMBOL_PARAMETER; role <= SYMBOL_RESULT; role++) {
            if (fprintf(output, "struct %s {\n", type->names[role == SYMBOL_PARAMETER
                    ? SCAFFOLD_PARAMETERS : SCAFFOLD_RESULTS]->internal_name) < 0
                || !fields(scaffold, output, type, role == SYMBOL_RESULT, true, role)
                || fputs("};\n", output) == EOF) {
                return false;
            }
        }
    }
    return fputs("#endif\n", output) != EOF;
}
