/** @file
 * @brief Materialize compiler-known frame offsets without C-layout arithmetic.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) keeps caller slots
 * stable; [LIST-04](../../../../spec/language/lists.md#list-04) separates lexical
 * method/service metadata from declared Ama fields and native payloads.
 */
#include "lists.h"

bool codegen_list_offset(FILE *output, const ScaffoldType *type, const StorageFrame *frame) {
    const char *name = type->names[SCAFFOLD_WRITE]->internal_name;
    switch (frame->statement->operation_kind) {
        case SEMANTIC_LIST_SERVICE_OPERATION:
            return fprintf(output, "offsetof(%s_Storage,list_service_%zu)", name, frame->slot) >= 0;
        case SEMANTIC_LIST_METHOD_OPERATION:
            return fprintf(output, "offsetof(%s_Storage,list_method_%zu)", name, frame->slot) >= 0;
        case SEMANTIC_INSTANCE_OPERATION:
            return fprintf(output, "offsetof(%s_Storage,operation_frames[%zu])", name, frame->slot) >= 0;
        default: return false;
    }
}

bool codegen_list_frame(FILE *output, const ScaffoldType *type,
    const StorageFrame *frame, bool method, const char *variable) {
    if (frame == NULL || frame->statement->operation_kind != (method
            ? SEMANTIC_LIST_METHOD_OPERATION : SEMANTIC_LIST_SERVICE_OPERATION)) { return false; }
    return fprintf(output, "%s *%s=NULL; ama_status=%s(&ama_program,ama_current,",
            method ? "AmaProgramListMethod" : "AmaProgramListService", variable,
            method ? "ama_program_list_method_frame" : "ama_program_list_service_frame") >= 0
        && codegen_list_offset(output, type, frame)
        && fprintf(output, ",&%s); if(ama_status.code!=AMA_OK) goto ama_project_finish;\n", variable) >= 0;
}
