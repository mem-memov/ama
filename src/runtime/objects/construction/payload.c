/**
 * @file
 * @brief Invoke exactly the selected payload constructor after owned children are ready.
 *
 * [LIFE-02](../../../../spec/implementation/native-interface.md#life-02) requires explicit lifecycle ordering.
 * Only successful initialization marks the native payload ready for destruction.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) preserves stable instance storage.
 * [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03) constructs
 * a fixed integer length directly from host-size metadata, after owned fields.
 * Raw zero-code completion receives its cleanup credit before checked transport can fail.
 * [ERROR-03](../../../../spec/implementation/diagnostics.md#error-03) requires
 * malformed status fields to become technical failure without losing ownership.
 */
#include "internal.h"
#include "runtime/lifecycle/native.h"

AmaStatus ama_construction_payload(AmaObjectOperation *operation, AmaObjectWork *work) {
    operation->world->construction_operation = work->operation;
    if (work->source.generation != 0) { return ama_object_clone_payload(operation, work, work->clone_root->continuation); }
    AmaObjectType *type = work->type;
    void *payload = (unsigned char *)ama_pool_payload(&type->pool, work->cell) + type->native_offset;
    AmaConstructorKind kind = work->recipe == NULL ? AMA_CONSTRUCT_DEFAULT : work->recipe->constructor;
    AmaStatus status = ama_status_success();
    if (kind == AMA_CONSTRUCT_DEFAULT) {
        if (type->hooks.initialize != NULL) { status = type->hooks.initialize(payload); }
    } else if (kind == AMA_CONSTRUCT_LITERAL && type->hooks.literal != NULL) {
        status = type->hooks.literal(payload, work->recipe->literal);
    } else if (kind == AMA_CONSTRUCT_CLONE && work->recipe->native_source != NULL) {
        if (type->hooks.initialize != NULL && type->hooks.clone == NULL) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
        if (type->hooks.clone != NULL) { status = type->hooks.clone(payload, work->recipe->native_source); }
    } else if (kind == AMA_CONSTRUCT_SIZE && type->hooks.initialize != NULL && work->recipe->from_size != NULL) {
        status = work->recipe->from_size(payload, work->recipe->size_value);
    } else { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (status.code == AMA_OK) { work->cell->metadata.service.activity = AMA_INSTANCE_READY; }
    return ama_lifetime_finish_native(&work->cell->metadata.lifetime,
        type->hooks.initialize != NULL, status);
}
