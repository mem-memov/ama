#include "runtime/program/program.h"

const AmaProgramType *ama_program_type(const AmaProgram *program, uint64_t id) {
    for (size_t index = 0; index < program->type_count; index++) {
        const AmaProgramType *type = &program->types[index];
        if (type->object != NULL && type->object->pool.type_id == id) { return type; }
    }
    return NULL;
}

static AmaStatus ama_program_shared_base(AmaProgram *program, const AmaProgramAccess *access,
    const AmaConstantValue *current, AmaProgramValue *value) {
    const AmaConstantField *field;
    if (access->shared) {
        if (access->field >= program->constant_count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        const AmaProgramConstant *declaration = &program->constants[access->field];
        field = declaration->storage;
        value->type_id = declaration->type_id;
        value->form = declaration->list ? AMA_FIELD_OWNING_LIST : AMA_FIELD_OWNED;
    } else {
        if (current == NULL || access->field >= current->type->field_count) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        const AmaObjectField *descriptor = &current->type->object->fields[access->field];
        field = &current->fields[access->field];
        value->type_id = descriptor->type_id;
        value->form = descriptor->kind;
    }
    if (value->form == AMA_FIELD_OWNING_LIST || value->form == AMA_FIELD_REFERENCE_LIST) {
        value->shared_list = field;
    } else if (value->form == AMA_FIELD_OWNED) {
        if (field->count != 1 || field->items == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        value->constant = field->items;
    }
    return ama_status_success();
}

static AmaStatus ama_program_instance_base(AmaProgram *program, AmaHandle current,
    const AmaProgramAccess *access, bool private_access, AmaProgramValue *value) {
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
    if (field->kind == AMA_FIELD_OWNING_LIST || field->kind == AMA_FIELD_REFERENCE_LIST) { value->list = address; }
    else { value->location = address; value->handle = *value->location; }
    return ama_status_success();
}

AmaStatus ama_program_payload(AmaProgram *program, const AmaProgramValue *value, const void **output) {
    if (value->form != AMA_FIELD_OWNED && value->form != AMA_FIELD_REFERENCE) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    if (value->constant != NULL) { *output = value->constant->payload; return ama_status_success(); }
    AmaObjectType *type = ama_world_type(program->world, value->type_id);
    if (type == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(&type->pool, value->handle, value->private_access, &cell);
    if (status.code != AMA_OK) { return status; }
    status = cell->metadata.identity.live ? ama_instance_access(&cell->metadata, AMA_ACCESS_READ)
        : cell->metadata.lifetime.phase != AMA_LIFETIME_LIVE ? ama_status_make(AMA_INVALID_STATE, NULL, 0)
        : ama_service_access(&cell->metadata.service, AMA_ACCESS_READ);
    if (status.code == AMA_OK) { *output = (unsigned char *)ama_pool_payload(&type->pool, cell) + type->native_offset; }
    return status;
}

static AmaStatus ama_program_index(AmaProgramValue *value, size_t index) {
    bool reference = value->form == AMA_FIELD_REFERENCE_LIST;
    if (value->form != AMA_FIELD_OWNING_LIST && !reference) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    size_t count = value->shared_list != NULL ? value->shared_list->count : value->list->count;
    if (index >= count) { return ama_status_make(AMA_INDEX_BOUNDS, NULL, 0); }
    if (value->shared_list != NULL) {
        if (!reference) { value->constant = &value->shared_list->items[index]; }
        value->shared_list = NULL;
    } else { value->location = &value->list->items[index]; value->handle = *value->location; value->list = NULL; }
    value->form = reference ? AMA_FIELD_REFERENCE : AMA_FIELD_OWNED;
    return ama_status_success();
}

static AmaStatus ama_program_access(AmaProgram *program, AmaHandle current, const AmaConstantValue *shared,
    const AmaProgramAccess *access, bool private_access, AmaProgramValue *output, bool *selection_refusal) {
    if (selection_refusal != NULL) { *selection_refusal = false; }
    size_t depth = 0;
    for (const AmaProgramAccess *node = access; node != NULL; node = node->index) { depth++; }
    size_t selected = 0;
    while (depth != 0) {
        depth--;
        const AmaProgramAccess *node = access;
        for (size_t index = 0; index < depth; index++) { node = node->index; }
        AmaProgramValue value = {0};
        AmaStatus status = node->shared || shared != NULL ? ama_program_shared_base(program, node, shared, &value)
            : ama_program_instance_base(program, current, node, private_access, &value);
        if (status.code == AMA_OK && node->index != NULL) {
            status = ama_program_index(&value, selected);
            if (status.code == AMA_INDEX_BOUNDS && selection_refusal != NULL) { *selection_refusal = true; }
        }
        if (status.code != AMA_OK) { return status; }
        if (depth == 0) { *output = value; return ama_status_success(); }
        const AmaProgramType *type = ama_program_type(program, value.type_id);
        const void *payload;
        status = ama_program_payload(program, &value, &payload);
        if (status.code != AMA_OK) { return status; }
        if (type == NULL || type->index == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
        status = type->index(payload, &selected);
        if (status.code != AMA_OK) { return status; }
    }
    return ama_status_make(AMA_INVALID_STATE, NULL, 0);
}

AmaStatus ama_program_select(AmaProgram *program, AmaHandle current, const AmaProgramAccess *access,
    bool private_access, AmaProgramValue *output) {
    return ama_program_access(program, current, NULL, access, private_access, output, NULL);
}

AmaStatus ama_program_select_shared(AmaProgram *program, const AmaConstantValue *current,
    const AmaProgramAccess *access, AmaProgramValue *output) {
    return ama_program_access(program, (AmaHandle){0}, current, access, false, output, NULL);
}

AmaStatus ama_program_evaluate(AmaProgram *program, AmaHandle current, const AmaConstantValue *shared,
    const AmaProgramSource *source, bool private_access, AmaProgramValue *output) {
    if (source->kind == AMA_SOURCE_LITERAL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    AmaProgramAccess global = {.field = source->shared, .shared = true};
    const AmaProgramAccess *access = source->kind == AMA_SOURCE_SHARED ? &global : source->access;
    AmaStatus status = ama_program_access(program, current, shared, access, private_access, output, NULL);
    if (status.code == AMA_OK && source->acquire && output->constant != NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (status.code == AMA_OK && source->acquire) { output->form = AMA_FIELD_REFERENCE; }
    if (status.code == AMA_OK) { output->continuation = source->continuation; }
    return status;
}

AmaStatus ama_program_select_target(AmaProgram *program, AmaHandle current, const AmaProgramAccess *access,
    AmaProgramValue *output, bool *selection_refusal) {
    return ama_program_access(program, current, NULL, access, false, output, selection_refusal);
}
