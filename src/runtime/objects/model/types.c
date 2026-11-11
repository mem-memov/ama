#include <stdint.h>
#include "runtime/objects/internal.h"

bool ama_field_owns(AmaFieldKind kind) {
    return kind == AMA_FIELD_OWNED || kind == AMA_FIELD_OWNING_LIST;
}

bool ama_field_list(AmaFieldKind kind) {
    return kind == AMA_FIELD_OWNING_LIST || kind == AMA_FIELD_REFERENCE_LIST;
}

AmaStatus ama_object_type_init(AmaObjectType *type, SourceAllocator allocator, uint64_t id,
    size_t size, size_t slots, uint64_t limit, const AmaObjectField *fields, size_t count,
    size_t native_offset, AmaPayloadHooks hooks) {
    *type = (AmaObjectType){0};
    if (native_offset > size || (hooks.initialize == NULL) != (hooks.destroy == NULL)
        || (hooks.initialize == NULL && (hooks.literal != NULL || hooks.clone != NULL))) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    for (size_t index = 0; index < count; index++) {
        const AmaObjectField *field = &fields[index];
        size_t width = ama_field_list(field->kind) ? sizeof(AmaHandleList) : sizeof(AmaHandle);
        size_t alignment = ama_field_list(field->kind) ? _Alignof(AmaHandleList) : _Alignof(AmaHandle);
        if (field->kind > AMA_FIELD_REFERENCE_LIST || field->type_id == 0
            || field->offset % alignment != 0 || field->offset > size || width > size - field->offset
            || (hooks.initialize != NULL && field->offset + width > native_offset)) {
            return ama_status_make(AMA_INVALID_STATE, NULL, 0);
        }
        for (size_t previous = 0; previous < index; previous++) {
            size_t other = ama_field_list(fields[previous].kind) ? sizeof(AmaHandleList) : sizeof(AmaHandle);
            if (field->offset < fields[previous].offset + other && fields[previous].offset < field->offset + width) {
                return ama_status_make(AMA_INVALID_STATE, NULL, 0);
            }
        }
    }
    AmaStatus status = ama_pool_init(&type->pool, allocator, id, size, slots, limit);
    if (status.code == AMA_OK) {
        type->fields = fields;
        type->field_count = count;
        type->native_offset = native_offset;
        type->hooks = hooks;
    }
    return status;
}

AmaObjectType *ama_world_type(const AmaWorld *world, uint64_t id) {
    for (size_t index = 0; index < world->type_count; index++) {
        if (world->types[index]->pool.type_id == id) {
            return world->types[index];
        }
    }
    return NULL;
}

bool ama_same_handle(AmaHandle left, AmaHandle right) {
    return left.type_id == right.type_id && left.slot_id == right.slot_id && left.generation == right.generation;
}

void ama_object_report(AmaWorld *world, AmaStatus status) {
    if (status.code != AMA_OK && world->report != NULL) {
        world->report(world->report_context, status);
    }
}

AmaStatus ama_object_lookup(AmaWorld *world, AmaHandle handle, bool private_slot,
    AmaObjectType **type, AmaPoolCell **cell) {
    if (handle.generation == 0) {
        return ama_status_make(AMA_EMPTY_REFERENCE, NULL, 0);
    }
    *type = ama_world_type(world, handle.type_id);
    return *type == NULL ? ama_status_make(AMA_TYPE_MISMATCH, NULL, 0)
        : ama_pool_lookup(&(*type)->pool, handle, private_slot, cell);
}
