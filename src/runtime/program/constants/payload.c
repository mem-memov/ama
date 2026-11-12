/**
 * @file
 * @brief Construct one aligned shared native payload with exactly one selected hook.
 *
 * [CONST-02](../../../../spec/language/declarations.md#const-02)
 * Record the allocation before invoking native code; only a successful hook creates a destructor credit.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"

/** Record an aligned allocation, then invoke exactly clone, literal or default hook. Only success creates native_live; failed constructor cleanup remains the hook contract. */
AmaStatus ama_constant_payload(AmaProgram *program, AmaConstantBuildWork *work) {
    AmaObjectType *type = work->value->type->object;
    program->world->construction_operation = work->operation;
    size_t size = type->pool.payload_size - type->native_offset;
    size_t alignment = type->pool.alignment;
    if (size == 0) { size = 1; }
    if (size > SIZE_MAX - (alignment - 1)) { return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); }
    void *allocation = program->world->allocator.allocate(program->world->allocator.context, size + alignment - 1);
    if (allocation == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    size_t padding = (alignment - (uintptr_t)allocation % alignment) % alignment;
    work->value->allocation = allocation;
    work->value->payload = (unsigned char *)allocation + padding;
    memset(work->value->payload, 0, size);
    AmaStatus status = ama_status_success();
    if (type->hooks.initialize == NULL) { return status; }
    if (work->source != NULL) {
        status = type->hooks.clone == NULL ? ama_status_make(AMA_INVALID_STATE, NULL, 0)
            : type->hooks.clone(work->value->payload, work->source->payload);
    } else if (work->initializer != NULL && work->initializer->kind == AMA_SOURCE_LITERAL) {
        status = type->hooks.literal == NULL ? ama_status_make(AMA_INVALID_STATE, NULL, 0)
            : type->hooks.literal(work->value->payload, work->initializer->literal);
    } else { status = type->hooks.initialize(work->value->payload); }
    work->value->native_live = status.code == AMA_OK;
    return status;
}

