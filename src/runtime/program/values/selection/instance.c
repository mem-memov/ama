/**
 * @file
 * @brief Select declared scalar/list locations inside one checked instance.
 *
 * [LIST-10](../../../../../spec/language/lists.md#list-10) allows only an exact structural grant to select busy list storage.
 * [REF-01](../../../../../spec/language/values-and-references.md#ref-01)
 * keeps declared storage form distinct from the current referenced identity.
 */
#include "internal.h"
#include "runtime/program/lists/service.h"

/** Resolve one checked field under optional exact list authority. Only an actual
 * denied list acquisition sets list_refusal; other failures remain technical.
 * The caller uses a private value until the enclosing full selection succeeds.
 */
AmaStatus ama_program_instance_authorized(AmaProgram *program, AmaHandle current,
    const AmaProgramAccess *access, bool private_access, const AmaProgramListService *authority,
    bool *list_refusal, AmaProgramValue *value) {
    if (list_refusal != NULL) { *list_refusal = false; }
    AmaObjectType *type = ama_world_type(program->world, current.type_id);
    if (type == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(&type->pool, current, private_access, &cell);
    if (status.code != AMA_OK) { return status; }
    if (access->field >= type->field_count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    const AmaObjectField *field = &type->fields[access->field];
    void *address = (unsigned char *)ama_pool_payload(&type->pool, cell) + field->offset;
    value->type_id = field->type_id;
    value->form = field->kind;
    value->container = current;
    value->private_access = private_access;
    if (field->kind == AMA_FIELD_OWNING_LIST || field->kind == AMA_FIELD_REFERENCE_LIST) {
        AmaHandleList *list = address;
        if (list->acquisition != NULL) {
            status = ama_program_list_authorize(authority, program, current, access->field, list);
            if (status.code != AMA_OK) {
                if (list_refusal != NULL) { *list_refusal = true; }
                return status;
            }
        }
        value->list = list;
    }
    else { value->location = address; value->handle = *value->location; }
    return ama_status_success();
}

AmaStatus ama_program_instance_base(AmaProgram *program, AmaHandle current,
    const AmaProgramAccess *access, bool private_access, AmaProgramValue *value) {
    return ama_program_instance_authorized(program, current, access, private_access, NULL, NULL, value);
}
