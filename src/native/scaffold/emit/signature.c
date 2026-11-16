/**
 * @file
 * @brief Write exact six-protocol C signatures and validated include directives.
 *
 * [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03)
 * Signature spelling depends only on a valid protocol operation and validated local function bytes, not a whole type model.
 */
#include "native/scaffold/internal.h"

/** Write validated original local identifier bytes inside AMA_NATIVE without decoding, transliterating or expanding a different helper name.
 * Component contract: [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03). */
static bool function_name(FILE *output, SourceSpan name) {
    return fputs("AMA_NATIVE(", output) != EOF
        && fwrite(source_bytes(name.source) + name.start, 1, name.end - name.start, output) == name.end - name.start
        && fputc(')', output) != EOF;
}

bool scaffold_emit_signature(FILE *output, AbiOperation operation, SourceSpan function) {
    if (fputs("AmaStatus ", output) == EOF || !function_name(output, function)) {
        return false;
    }
    const char *signature = operation == ABI_DEFAULT ? "(AMA_NATIVE(State) *destination)"
        : operation == ABI_DESTROY ? "(AMA_NATIVE(State) *state)"
        : operation == ABI_LITERAL ? "(AMA_NATIVE(State) *destination, AmaByteView literal)"
        : operation == ABI_CLONE ? "(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source)"
        : operation == ABI_CONDITION ? "(const AMA_NATIVE(State) *state, bool *result)"
        : "(const AMA_NATIVE(State) *state, size_t *result)";
    return fputs(signature, output) != EOF;
}

bool scaffold_include(FILE *output, const char *path) {
    return fprintf(output, "#include \"%s\"\n", path) >= 0;
}
