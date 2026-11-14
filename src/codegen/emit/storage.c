/**
 * @file
 * @brief Emit declared handle/list fields followed by actual native payload storage.
 *
 * [RT-02](../../../spec/implementation/runtime.md#rt-02)
 * Types without pools allocate no instance storage. Mutable type descriptors belong to the owning runtime context.
 * [LIST-04](../../../spec/language/lists.md#list-04) assigns lexical intrinsic result cells.
 * [CALL-09](../../../spec/language/calls-and-results.md#call-09) stores pending
 * chain protections outside declared fields and native clone payloads.
 */
#include "codegen/internal.h"

/**
 * Write one C storage structure per pool-backed type: declared handles/lists followed by actual
 * native State or a reserved Ama payload byte. No default instance is emitted.
 */
bool codegen_emit_storage(NativeScaffold *scaffold, FILE *output) {
    if (fputs("#include \"runtime/program/lists/service.h\"\n", output) == EOF) { return false; }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (!type->plan->requires_pool) { continue; }
        const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
        if (fprintf(output, "typedef struct %s_Storage {\n", name) < 0) { return false; }
        for (const ScaffoldMember *member = type->members; member != NULL; member = member->next) {
            const char *field_type = member->field->symbol->list ? "AmaHandleList" : "AmaHandle";
            if (fprintf(output, "    %s %s;\n", field_type, member->identity->internal_name) < 0) { return false; }
        }
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE) {
            if (fprintf(output, "    %s_State payload;\n", type->native_prefix->internal_name) < 0) { return false; }
        } else if (fputs("    unsigned char payload;\n", output) == EOF) { return false; }
        /* Transient control slots have no declared field/native descriptor, so
         * ordinary and continuation cloning cannot copy pending chain grants.
         * [CALL-09](../../../spec/language/calls-and-results.md#call-09). */
        if (type->plan->frame_count != 0
            && fprintf(output, "    AmaProgramFrame operation_frames[%zu];\n", type->plan->frame_count) < 0) { return false; }
        /* Only actual lexical intrinsic sites reserve list-specific frames.
         * [LIST-04](../../../spec/language/lists.md#list-04) gives temporary
         * results explicit stable control storage outside copied Ama fields. */
        for (const StorageFrame *frame = type->plan->frames; frame != NULL; frame = frame->next) {
            SemanticOperationKind kind = frame->statement->operation_kind;
            if (kind == SEMANTIC_LIST_SERVICE_OPERATION && fprintf(output,
                    "    AmaProgramListService list_service_%zu;\n", frame->slot) < 0) { return false; }
            if (kind == SEMANTIC_LIST_METHOD_OPERATION && fprintf(output,
                    "    AmaProgramListMethod list_method_%zu;\n", frame->slot) < 0) { return false; }
        }
        if (fprintf(output, "} %s_Storage;\n", name) < 0) { return false; }
    }
    return true;
}
