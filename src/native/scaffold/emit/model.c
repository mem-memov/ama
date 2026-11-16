/** @file
 * @brief Export exact authoring role identities without filename/hash inference.
 * [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06) provides
 * generated native interfaces. Their machine-readable role map lets build tools
 * bind authored companions to those same checked identities and filenames.
 */
#include "native/scaffold/internal.h"

/** Write the complete framed identity alongside its candidate C spelling. Component contract: [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06). */
static bool identity(FILE *output, const IdentityName *name) {
    static const char hex[] = "0123456789abcdef";
    if (fputs("{\"name\":", output) == EOF || !scaffold_json(output, name->internal_name)
        || fputs(",\"key_hex\":\"", output) == EOF) {
        return false;
    }
    for (size_t index = 0; index < name->encoding_length; index++) {
        unsigned char byte = name->encoding[index];
        if (fputc(hex[byte >> 4], output) == EOF || fputc(hex[byte & 15], output) == EOF) {
            return false;
        }
    }
    return fputs("\"}", output) != EOF;
}

/** Serialize one exact native type; paths are generated basenames, not discovery candidates. Component contract: [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06). */
static bool module(FILE *output, const ScaffoldType *type) {
    return fputs("{\"module\":", output) != EOF
        && scaffold_json(output, type->plan->type->module->source->logical_path)
        && fputs(",\"namespace\":", output) != EOF && identity(output, type->native_prefix)
        && fputs(",\"parameters\":", output) != EOF && identity(output, type->names[SCAFFOLD_PARAMETERS])
        && fputs(",\"results\":", output) != EOF && identity(output, type->names[SCAFFOLD_RESULTS])
        && fputs(",\"interface\":", output) != EOF && scaffold_json(output, type->interface_name)
        && fputs(",\"begin\":", output) != EOF && scaffold_json(output, type->begin_name)
        && fputs(",\"end\":", output) != EOF && scaffold_json(output, type->end_name)
        && fputc('}', output) != EOF;
}

bool scaffold_emit_model(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    if (fputs("{\"format\":\"ama-native-model-1\",\"modules\":[", output) == EOF) {
        return false;
    }
    bool first = true;
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) {
            continue;
        }
        if ((!first && fputc(',', output) == EOF) || !module(output, type)) {
            return false;
        }
        first = false;
    }
    return fputs("]}\n", output) != EOF;
}
