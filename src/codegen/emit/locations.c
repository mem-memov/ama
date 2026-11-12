/**
 * @file
 * @brief Emit exact operation/declaration source-map selectors for runtime diagnostics.
 *
 * [DEBUG-01](../../../spec/implementation/diagnostics.md#debug-01)
 * Byte-octal string emission preserves original path/name bytes without C escape ambiguity; selector exhaustion fails generation.
 */
#include <stdint.h>
#include "codegen/internal.h"

/**
 * Find a declaration by exact symbol identity after the allocated control-selector range. Check
 * increment exhaustion and return zero on missing/failed selection; repeated scans are exact.
 */
size_t codegen_declaration_selector(NativeScaffold *scaffold, const SemanticSymbol *symbol) {
    size_t selector = scaffold->frontend->plan.next_selector;
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        for (const SemanticSymbol *field = type->plan->type->first; field != NULL; field = field->next) {
            if (selector == SIZE_MAX) {
                scaffold_error(scaffold, DIAGNOSTIC_OUT_OF_MEMORY, "The source-map selector space is exhausted", NULL);
                return 0;
            }
            selector++;
            if (field == symbol) { return selector; }
        }
    }
    return 0;
}

/**
 * Write one nonzero selector location using source position and octal-escaped path/type bytes.
 * Missing source emits no row; invalid position or IO fails.
 */
static bool codegen_location(FILE *output, size_t selector, SourceSpan span, const char *role, const SemanticType *type) {
    if (selector == 0) { return false; }
    if (span.source == NULL) { return true; }
    SourcePosition position;
    if (!source_position(span.source, span.start, &position)
        || fprintf(output, "{.operation=UINT64_C(%zu),.path=\"", selector) < 0) { return false; }
    const unsigned char *path = (const unsigned char *)source_path(span.source);
    for (; *path != 0; path++) { if (fprintf(output, "\\%03o", (unsigned)*path) < 0) { return false; } }
    if (fprintf(output, "\",.line=%zu,.column=%zu,.role=\"%s\",.type=\"", position.line, position.column, role) < 0) { return false; }
    path = (const unsigned char *)type->module->source->logical_path;
    for (; *path != 0; path++) { if (fprintf(output, "\\%03o", (unsigned)*path) < 0) { return false; } }
    return fputs("\"},\n", output) != EOF;
}

/**
 * Emit control and declaration locations in model order, preserving exact operation selectors
 * and source-backed roles. The generated table borrows immutable compiled strings.
 */
bool codegen_emit_locations(NativeScaffold *scaffold, FILE *output) {
    if (fputs("#include \"runtime/program/diagnostics/trace.h\"\nstatic const AmaProgramLocation ama_project_locations[] = {\n", output) == EOF) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        for (const ControlNode *node = type->plan->controls; node != NULL; node = node->allocated_next) {
            SourceSpan span = node->site.source == NULL ? (SourceSpan){.source=type->plan->type->module->syntax.layout.source} : node->site;
            const char *role = node->operation == CONTROL_NATIVE_ENTRY ? "native entry" : node->binding != NULL ? "binding"
                : node->field != NULL ? "interface construction" : "Ama operation";
            if (!codegen_location(output, node->selector, span, role, node->value_type == NULL ? type->plan->type : node->value_type)) { return false; }
        }
        for (const SemanticSymbol *field = type->plan->type->first; field != NULL; field = field->next) {
            if (!codegen_location(output, codegen_declaration_selector(scaffold, field), field->declaration->span,
                    field->role == SYMBOL_CONSTANT ? "shared construction" : "field construction", field->type)) { return false; }
        }
    }
    return fputs("};\n", output) != EOF;
}
