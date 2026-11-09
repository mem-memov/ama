#include "runtime/value.h"
#include <stdbool.h>
#include <stdlib.h>

AmaStatus ama_type_validate(const AmaType* type) {
    if (!type) return AMA_STATUS_INVALID;
    if (type->abi_version != AMA_VALUE_ABI_VERSION) return AMA_STATUS_ABI_MISMATCH;
    if (!type->module_path || !type->module_path[0] || !type->size ||
        !type->alignment || type->alignment > _Alignof(max_align_t) ||
        (type->alignment & (type->alignment - 1)) || type->size % type->alignment ||
        !type->initialize || !type->move || !type->destroy)
        return AMA_STATUS_INVALID;
    return AMA_STATUS_OK;
}

static AmaStatus validate_value(AmaValue value) {
    AmaStatus status = ama_type_validate(value.type);
    if (status != AMA_STATUS_OK) return status;
    if (!value.state || (uintptr_t)value.state % value.type->alignment)
        return AMA_STATUS_INVALID;
    return AMA_STATUS_OK;
}

static AmaStatus validate_pair(AmaValue destination, AmaValue source) {
    AmaStatus status = validate_value(destination);
    if (status != AMA_STATUS_OK) return status;
    status = validate_value(source);
    if (status != AMA_STATUS_OK) return status;
    if (destination.type != source.type) return AMA_STATUS_TYPE_MISMATCH;
    uintptr_t destination_address = (uintptr_t)destination.state;
    uintptr_t source_address = (uintptr_t)source.state;
    uintptr_t distance = destination_address > source_address
        ? destination_address - source_address : source_address - destination_address;
    return distance < destination.type->size ? AMA_STATUS_INVALID : AMA_STATUS_OK;
}

AmaStatus ama_value_initialize(AmaValue destination) {
    AmaStatus status = validate_value(destination);
    if (status != AMA_STATUS_OK) return status;
    return destination.type->initialize(destination.type, destination.state);
}

AmaStatus ama_value_initialize_literal(AmaValue destination, const unsigned char* bytes, size_t length) {
    AmaStatus status = validate_value(destination);
    if (status != AMA_STATUS_OK) return status;
    if (!bytes && length) return AMA_STATUS_INVALID;
    if (!destination.type->initialize_literal) return AMA_STATUS_LITERAL_UNSUPPORTED;
    return destination.type->initialize_literal(destination.type, destination.state, bytes, length);
}

AmaStatus ama_value_initialize_copy(AmaValue destination, AmaValue source) {
    AmaStatus status = validate_pair(destination, source);
    if (status != AMA_STATUS_OK) return status;
    if (!destination.type->initialize_copy) return AMA_STATUS_COPY_UNSUPPORTED;
    return destination.type->initialize_copy(destination.type, destination.state, source.state);
}

AmaStatus ama_value_move(AmaValue destination, AmaValue source) {
    AmaStatus status = validate_pair(destination, source);
    if (status != AMA_STATUS_OK) return status;
    destination.type->move(destination.type, destination.state, source.state);
    return AMA_STATUS_OK;
}

void ama_value_destroy(AmaValue value) {
    value.type->destroy(value.type, value.state);
}

static void commit_value(AmaValue destination, void* prepared) {
    ama_value_destroy(destination);
    destination.type->move(destination.type, destination.state, prepared);
    free(prepared);
}

AmaStatus ama_value_replace(AmaValue destination, AmaValue source) {
    if (destination.state == source.state && destination.type == source.type)
        return validate_value(destination);
    AmaStatus status = validate_pair(destination, source);
    if (status != AMA_STATUS_OK) return status;
    if (!destination.type->initialize_copy) return AMA_STATUS_COPY_UNSUPPORTED;
    void* prepared = malloc(destination.type->size);
    if (!prepared) return AMA_STATUS_MEMORY;
    status = destination.type->initialize_copy(destination.type, prepared, source.state);
    if (status == AMA_STATUS_OK) commit_value(destination, prepared);
    else free(prepared);
    return status;
}

static AmaStatus replace_constructed(AmaValue destination, const unsigned char* bytes,
    size_t length, bool literal) {
    AmaStatus status = validate_value(destination);
    if (status != AMA_STATUS_OK) return status;
    if (literal && !bytes && length) return AMA_STATUS_INVALID;
    if (literal && !destination.type->initialize_literal) return AMA_STATUS_LITERAL_UNSUPPORTED;
    void* prepared = malloc(destination.type->size);
    if (!prepared) return AMA_STATUS_MEMORY;
    status = literal
        ? destination.type->initialize_literal(destination.type, prepared, bytes, length)
        : destination.type->initialize(destination.type, prepared);
    if (status == AMA_STATUS_OK) commit_value(destination, prepared);
    else free(prepared);
    return status;
}

AmaStatus ama_value_replace_literal(AmaValue destination, const unsigned char* bytes, size_t length) {
    return replace_constructed(destination, bytes, length, true);
}

AmaStatus ama_value_reset(AmaValue destination) {
    return replace_constructed(destination, NULL, 0, false);
}
