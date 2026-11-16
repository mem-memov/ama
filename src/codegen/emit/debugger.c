/** @file
 * @brief Preserve original source names independently of generated C identifiers.
 * [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03) permits
 * inspection of declared state, never inferred native layouts or invented history.
 * The catalog follows the same storage and scope order as runtime descriptors.
 */
#include <string.h>
#include "codegen/internal.h"

/** Emit exact bytes as octal C escapes, avoiding locale and escape adjacency. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03). */
static bool debug_name(FILE *output, const unsigned char *bytes, size_t length) {
    if (fputc('"', output) == EOF) { return false; }
    for (size_t index = 0; index < length; index++) {
        if (fprintf(output, "\\%03o", (unsigned)bytes[index]) < 0) { return false; }
    }
    return fputc('"', output) != EOF;
}

/** Emit a validated semantic name without normalization or synthetic spelling. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03). */
static bool debug_symbol(FILE *output, const SemanticSymbol *symbol) {
    SourceSpan span = symbol->name;
    return debug_name(output, source_bytes(span.source) + span.start, span.end - span.start);
}

/** Emit names for each pooled type's declared fields and lexical traversal roles. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03). */
static bool debug_members(NativeScaffold *scaffold, FILE *output, const ScaffoldType *type) {
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    if (type->members != NULL) {
        if (fprintf(output, "static const AmaDebugField %s_DebugFields[] = {\n", name) < 0) { return false; }
        for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
            if (fputs("{.name=", output) == EOF || !debug_symbol(output, member->field->symbol)
                || fprintf(output, ",.operation=UINT64_C(%zu)},\n", codegen_declaration_selector(scaffold, member->field->symbol)) < 0) { return false; }
        }
        if (fputs("};\n", output) == EOF) { return false; }
    }
    if (type->plan->type->foreach_bindings != NULL) {
        if (fprintf(output, "static const char *const %s_DebugBindings[] = {\n", name) < 0) { return false; }
        for (const PlannedLoop *loop = type->plan->loops; loop != NULL; loop = loop->next) {
            if (loop->statement->syntax->kind != AST_FOREACH) { continue; }
            if (!debug_symbol(output, loop->statement->traversal->binding) || fputs(",\n", output) == EOF) { return false; }
        }
        if (fputs("};\n", output) == EOF) { return false; }
    }
    return true;
}

/** Publish immutable original names alongside exact source-map and nominal identities. Component contract: [DEBUG-03](../../../spec/implementation/diagnostics.md#debug-03). */
bool codegen_emit_debugger(NativeScaffold *scaffold, FILE *output) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->requires_pool && !debug_members(scaffold, output, type)) { return false; }
    }
    if (fputs("static const AmaDebugType ama_project_debug_types[] = {\n", output) == EOF) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (!type->plan->requires_pool) { continue; }
        const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
        const char *path = type->plan->type->module->source->logical_path;
        if (fprintf(output, "{.type_id=UINT64_C(%zu),.name=", codegen_type_id(scaffold, type)) < 0
            || !debug_name(output, (const unsigned char *)path, strlen(path))) { return false; }
        if (type->members != NULL && fprintf(output, ",.fields=%s_DebugFields,.field_count=%zu", name, type->plan->field_count) < 0) { return false; }
        if (type->plan->type->foreach_bindings != NULL
            && fprintf(output, ",.bindings=%s_DebugBindings,.binding_count=sizeof(%s_DebugBindings)/sizeof(%s_DebugBindings[0])", name, name, name) < 0) { return false; }
        if (fputs("},\n", output) == EOF) { return false; }
    }
    if (fputs("};\nstatic const AmaDebugConstant ama_project_debug_constants[] = {\n{0},\n", output) == EOF) { return false; }
    /* The leading sentinel makes the zero-constant case valid ISO C. */
    for (const SharedStorage *shared = scaffold->frontend->plan.shared_first; shared != NULL; shared = shared->next) {
        const SemanticSymbol *symbol = shared->constant->symbol;
        const char *owner = symbol->owner->module->source->logical_path;
        if (fputs("{.owner=", output) == EOF || !debug_name(output, (const unsigned char *)owner, strlen(owner))
            || fputs(",.name=", output) == EOF || !debug_symbol(output, symbol)
            || fprintf(output, ",.operation=UINT64_C(%zu)},\n", codegen_declaration_selector(scaffold, symbol)) < 0) { return false; }
    }
    return fprintf(output, "};\nstatic const AmaDebugCatalog ama_project_debug_catalog = {\n"
        ".locations=ama_project_locations,.location_count=sizeof(ama_project_locations)/sizeof(ama_project_locations[0]),\n"
        ".types=ama_project_debug_types,.type_count=sizeof(ama_project_debug_types)/sizeof(ama_project_debug_types[0]),\n"
        ".constants=ama_project_debug_constants+1,.constant_count=%zu};\n", scaffold->frontend->semantic.constant_count) >= 0;
}
