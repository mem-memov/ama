/** @file
 * @brief Read only compiler-declared handle/list fields, never arbitrary native payload bytes.
 * [REF-01](../../../../spec/language/values-and-references.md#ref-01) separates identity from storage;
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) supplies truthful field offsets.
 */
#include "../internal.h"
/** Declared field descriptors carry layout authority; native_offset is deliberately unused by this traversal. */
AmaStatus ama_debug_fields(AmaDebugSession *session, AmaContextReference reference, AmaDebugFieldVisitor visit, void *context) {
    if (visit == NULL) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    const AmaProgramType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_debug_resolve(session,reference,&type,&cell);
    if (status.code != AMA_OK) { return status; }
    status = ama_debug_visit_begin(session);
    if (status.code != AMA_OK) { return status; }
    const unsigned char *storage = ama_pool_payload(&type->object->pool,cell);
    const AmaDebugType *names = ama_debug_type(session,reference.local.type_id);
    for (size_t index = 0; index < type->field_count; index++) {
        const AmaObjectField *field = &type->object->fields[index];
        const AmaProgramField *declaration = &type->fields[index];
        AmaDebugFieldValue value = {.name=names == NULL ? NULL : names->fields[index].name,.field=index,
            .role=declaration->role,.position=declaration->position,.type_id=field->type_id,.form=field->kind};
        if (field->kind == AMA_FIELD_OWNED || field->kind == AMA_FIELD_REFERENCE) {
            value.value = ama_debug_reference(session,*(const AmaHandle *)(storage+field->offset));
        } else {
            const AmaHandleList *list = (const AmaHandleList *)(storage+field->offset);
            value.count = list->count;
            value.incarnation = list->incarnation;
            value.acquired = list->acquisition != NULL;
        }
        if (!visit(context,&value)) { break; }
    }
    ama_debug_visit_end(session);
    return ama_status_success();
}
/** Reading current membership order never invokes list methods, checks target availability or advances a saved boundary. */
AmaStatus ama_debug_elements(AmaDebugSession *session, AmaContextReference reference, size_t field,
    AmaDebugElementVisitor visit, void *context) {
    if (visit == NULL) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    const AmaProgramType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_debug_resolve(session,reference,&type,&cell);
    if (status.code != AMA_OK) { return status; }
    if (field >= type->field_count) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    const AmaObjectField *descriptor = &type->object->fields[field];
    if (descriptor->kind != AMA_FIELD_OWNING_LIST && descriptor->kind != AMA_FIELD_REFERENCE_LIST) {
        return ama_status_make(AMA_TYPE_MISMATCH,NULL,0);
    }
    status = ama_debug_visit_begin(session);
    if (status.code != AMA_OK) { return status; }
    const unsigned char *storage = ama_pool_payload(&type->object->pool,cell);
    const AmaHandleList *list = (const AmaHandleList *)(storage+descriptor->offset);
    for (size_t position = 0; position < list->count; position++) {
        AmaDebugElement element = {.position=position};
        AmaHandle target;
        status = ama_membership_at(list,position,&element.membership,&target);
        if (status.code != AMA_OK) { break; }
        element.target = ama_debug_reference(session,target);
        if (!visit(context,&element)) { break; }
    }
    ama_debug_visit_end(session);
    return status;
}
