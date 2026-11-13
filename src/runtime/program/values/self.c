/** @file
 * @brief Form a typed current-instance reference without reading or acquiring its payload.
 * [REF-01](../../../../spec/language/values-and-references.md#ref-01) makes
 * standalone & the lexical current identity. [RT-12](../../../../spec/implementation/runtime.md#rt-12)
 * permits initialization to copy its reserved final identity while forbidding
 * external access to constructing state. Trusted private evaluation supplies
 * that authority; ordinary evaluation still requires a published identity.
 */
#include "selection/internal.h"

/** Validate exact reserved/public identity and lifecycle, then publish only a reference value with no payload location or acquired target. */
AmaStatus ama_program_self(AmaProgram *program, AmaHandle current,
    const AmaProgramSource *source, bool private_access, AmaProgramValue *output) {
    if (source->type_id != current.type_id) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    AmaObjectType *type = ama_world_type(program->world, current.type_id);
    if (type == NULL) { return ama_status_make(AMA_TYPE_MISMATCH, NULL, 0); }
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(&type->pool, current, private_access, &cell);
    if (status.code != AMA_OK) { return status; }
    AmaLifetimePhase phase = cell->metadata.lifetime.phase;
    if (phase != AMA_LIFETIME_CONSTRUCTING && phase != AMA_LIFETIME_LIVE) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    *output = (AmaProgramValue){.type_id = current.type_id, .form = AMA_FIELD_REFERENCE,
        .handle = current, .private_access = private_access};
    return ama_status_success();
}
