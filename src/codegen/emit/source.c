/**
 * @file
 * @brief Emit resolved access chains, byte-exact literals and ordered initializer values.
 *
 * [REF-03](../../../spec/language/values-and-references.md#ref-03)
 * Do not recover names at runtime. Inner index descriptors precede users; literal length is independent of terminators.
 */
#include "codegen/internal.h"

/**
 * Append a checked role/ordinal suffix to a validated generated base. Return arena-owned text or
 * first scaffold error; no user-name identity is inferred from this local suffix.
 */
const char *codegen_indexed_name(NativeScaffold *scaffold, const char *base, const char *role, size_t index) {
    char suffix[80];
    int length = snprintf(suffix, sizeof(suffix), "_%s_%zu", role, index);
    if (length < 0 || (size_t)length >= sizeof(suffix)) {
        scaffold_error(scaffold, DIAGNOSTIC_INVALID_PATH, "A generated identifier is unrepresentable", NULL);
        return NULL;
    }
    return scaffold_join(scaffold, base, "", suffix);
}

/**
 * Emit a finite resolved nested index chain from inside out so each referenced descriptor
 * already exists. No C recursion; repeated exact rescans cost O(depth squared).
 */
bool codegen_access_definition(NativeScaffold *scaffold, FILE *output, const char *base, const SemanticAccess *access) {
    size_t depth = 0;
    for (const SemanticAccess *node = access; node != NULL; node = node->index) { depth++; }
    while (depth != 0) {
        depth--;
        const SemanticAccess *node = access;
        for (size_t index = 0; index < depth; index++) { node = node->index; }
        const char *name = codegen_indexed_name(scaffold, base, "Access", depth);
        bool shared = node->symbol->role == SYMBOL_CONSTANT;
        size_t slot;
        if (shared) { slot = node->symbol->shared->ordinal; }
        else {
            const ScaffoldType *owner = native_scaffold_type(scaffold, node->symbol->owner);
            slot = codegen_member(owner, node->symbol)->field->slot;
        }
        if (name == NULL || fprintf(output, "static const AmaProgramAccess %s = {.field=%zu, .shared=%s", name, slot, shared ? "true" : "false") < 0) { return false; }
        if (node->index != NULL) {
            const char *index = codegen_indexed_name(scaffold, base, "Access", depth + 1);
            if (index == NULL || fprintf(output, ", .index=&%s", index) < 0) { return false; }
        }
        if (fputs("};\n", output) == EOF) { return false; }
    }
    return true;
}

/**
 * Emit byte-array literal storage with explicit original length or resolved access descriptors.
 * Empty literals receive a C-array sentinel without changing language byte length.
 */
bool codegen_source_definition(NativeScaffold *scaffold, FILE *output, const char *name, const SemanticSource *source) {
    if (source->kind == AST_LITERAL_SOURCE) {
        SourceSpan payload = source->syntax->payload;
        if (fprintf(output, "static const unsigned char %s_Bytes[] = {", name) < 0) { return false; }
        if (payload.start == payload.end && fputc('0', output) == EOF) { return false; }
        for (size_t index = payload.start; index < payload.end; index++) {
            if (fprintf(output, "%u,", (unsigned)source_bytes(payload.source)[index]) < 0) { return false; }
        }
        return fputs("};\n", output) != EOF;
    }
    return source->access == NULL || codegen_access_definition(scaffold, output, name, source->access);
}

/**
 * Write one resolved literal/local/shared descriptor with explicit acquisition/continuation
 * flags and exact nominal type ordinal; names and sources remain borrowed during emission.
 */
bool codegen_source_value(NativeScaffold *scaffold, FILE *output, const char *name, const SemanticSource *source) {
    const char *kind = source->kind == AST_LITERAL_SOURCE ? "AMA_SOURCE_LITERAL"
        : source->constant != NULL ? "AMA_SOURCE_SHARED" : "AMA_SOURCE_LOCAL";
    if (fprintf(output, "{.kind=%s, .acquire=%s, .continuation=%s", kind,
            source->kind == AST_ACQUIRE_SOURCE ? "true" : "false", source->kind == AST_CONTINUATION_SOURCE ? "true" : "false") < 0) { return false; }
    if (source->type != NULL) {
        const ScaffoldType *type = native_scaffold_type(scaffold, source->type);
        if (fprintf(output, ", .type_id=UINT64_C(%zu)", codegen_type_id(scaffold, type)) < 0) { return false; }
    }
    if (source->kind == AST_LITERAL_SOURCE) {
        SourceSpan payload = source->syntax->payload;
        if (fprintf(output, ", .literal={%s_Bytes,%zu}", name, payload.end - payload.start) < 0) { return false; }
    } else if (source->constant != NULL) {
        if (fprintf(output, ", .shared=%zu", source->constant->shared->ordinal) < 0) { return false; }
    } else {
        const char *access = codegen_indexed_name(scaffold, name, "Access", 0);
        if (access == NULL || fprintf(output, ", .access=&%s", access) < 0) { return false; }
    }
    return fputc('}', output) != EOF;
}

/**
 * Emit all dependent source definitions, then the ordered source array. Absence emits no array;
 * partial IO/allocation failure leaves the artifact unpublished.
 */
bool codegen_initializers(NativeScaffold *scaffold, FILE *output, const char *base, const SemanticSymbol *symbol) {
    size_t index = 0;
    for (const SemanticInitializer *initializer = symbol->initializers; initializer != NULL; initializer = initializer->next, index++) {
        const char *name = codegen_indexed_name(scaffold, base, "Source", index);
        if (name == NULL || !codegen_source_definition(scaffold, output, name, initializer->source)) { return false; }
    }
    if (index == 0) { return true; }
    if (fprintf(output, "static const AmaProgramSource %s_Sources[] = {\n", base) < 0) { return false; }
    index = 0;
    for (const SemanticInitializer *initializer = symbol->initializers; initializer != NULL; initializer = initializer->next, index++) {
        const char *name = codegen_indexed_name(scaffold, base, "Source", index);
        if (name == NULL || !codegen_source_value(scaffold, output, name, initializer->source) || fputs(",\n", output) == EOF) { return false; }
    }
    return fputs("};\n", output) != EOF;
}
