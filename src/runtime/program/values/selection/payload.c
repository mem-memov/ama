/**
 * @file
 * @brief Borrow a readable native payload from shared or checked instance storage.
 *
 * [COPY-07](../../../../../spec/language/values-and-references.md#copy-07)
 * requires availability checks before data can be consumed.
 * [FLOW-05](../../../../../spec/language/lists.md#flow-05) Retained payload reads validate associated membership before consuming native data or invoking a decoder.
 */
#include "internal.h"
#include "runtime/program/lists/traversal/traversal.h"

/** Borrow a payload only after identity/activity checks, or directly from immutable shared storage. Output changes only on success. */
AmaStatus ama_program_payload_authorized(AmaProgram *program, const AmaProgramValue *value,
    const AmaProgramListService *authority, const void **output, bool *origin_refused) {
    bool refused = false;
    AmaStatus origin = ama_program_foreach_guard(program, value, authority, &refused);
    if (origin_refused != NULL) { *origin_refused = refused; }
    if (origin.code != AMA_OK) { return origin; }
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

/** Ordinary retained reads inherit no structural list token. */
AmaStatus ama_program_payload(AmaProgram *program, const AmaProgramValue *value, const void **output) {
    return ama_program_payload_authorized(program, value, NULL, output, NULL);
}
