#include "runtime/record.h"

static AmaStatus validate_record(const AmaType* type) {
    const AmaRecord* record = type->context;
    if (!record || (!record->fields && record->field_count)) return AMA_STATUS_INVALID;
    for (size_t index = 0; index < record->field_count; index++) {
        const AmaField* field = &record->fields[index];
        AmaStatus status = ama_type_validate(field->type);
        if (status != AMA_STATUS_OK) return status;
        if (field->offset > type->size || field->type->size > type->size - field->offset ||
            type->alignment < field->type->alignment || field->offset % field->type->alignment)
            return AMA_STATUS_INVALID;
        for (size_t earlier = 0; earlier < index; earlier++) {
            const AmaField* previous = &record->fields[earlier];
            if (field->offset < previous->offset + previous->type->size &&
                previous->offset < field->offset + field->type->size)
                return AMA_STATUS_INVALID;
        }
    }
    return AMA_STATUS_OK;
}

static void destroy_prefix(const AmaRecord* record, void* state, size_t initialized) {
    while (initialized) {
        const AmaField* field = &record->fields[--initialized];
        field->type->destroy(field->type, (unsigned char*)state + field->offset);
    }
}

AmaStatus ama_record_initialize(const AmaType* type, void* destination) {
    AmaStatus status = validate_record(type);
    if (status != AMA_STATUS_OK) return status;
    const AmaRecord* record = type->context;
    for (size_t index = 0; index < record->field_count; index++) {
        const AmaField* field = &record->fields[index];
        status = field->type->initialize(field->type, (unsigned char*)destination + field->offset);
        if (status != AMA_STATUS_OK) {
            destroy_prefix(record, destination, index);
            return status;
        }
    }
    return AMA_STATUS_OK;
}

AmaStatus ama_record_initialize_copy(const AmaType* type, void* destination, const void* source) {
    AmaStatus status = validate_record(type);
    if (status != AMA_STATUS_OK) return status;
    const AmaRecord* record = type->context;
    for (size_t index = 0; index < record->field_count; index++) {
        const AmaField* field = &record->fields[index];
        status = field->type->initialize_copy
            ? field->type->initialize_copy(field->type, (unsigned char*)destination + field->offset,
                (const unsigned char*)source + field->offset)
            : AMA_STATUS_COPY_UNSUPPORTED;
        if (status != AMA_STATUS_OK) {
            destroy_prefix(record, destination, index);
            return status;
        }
    }
    return AMA_STATUS_OK;
}

void ama_record_move(const AmaType* type, void* destination, void* source) {
    const AmaRecord* record = type->context;
    for (size_t index = 0; index < record->field_count; index++) {
        const AmaField* field = &record->fields[index];
        field->type->move(field->type, (unsigned char*)destination + field->offset,
            (unsigned char*)source + field->offset);
    }
}

void ama_record_destroy(const AmaType* type, void* state) {
    const AmaRecord* record = type->context;
    destroy_prefix(record, state, record->field_count);
}
