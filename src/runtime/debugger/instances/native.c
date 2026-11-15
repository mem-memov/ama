/** @file
 * @brief Preview native values only through explicit type-specific read adapters at an available instance.
 * [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03) keeps conversions explicit;
 * [PAR-05](../../../../spec/language/async-and-parallelism.md#par-05) leaves native reader lifetime/affinity with its provider.
 */
#include "../internal.h"
/** One synchronous read observation has no persistent borrow and cannot release its supplying session. */
typedef struct {
    AmaDebugNativeVisitor visit;
    void *context;
    AmaStatus status;
    bool emitted;
    bool stopped;
} DebugNativeVisit;
/** Native readers promise truthful typed data; check transport shape and canonical sign/magnitude before forwarding. */
static bool debug_native_shape(const AmaDebugNativeValue *value) {
    if (value == NULL || (value->bytes.data == NULL && value->bytes.length != 0)) { return false; }
    if (value->kind == AMA_DEBUG_BOOLEAN) {
        return value->bytes.data == NULL && value->bytes.length == 0 && !value->negative && !value->little_endian;
    }
    if (value->kind == AMA_DEBUG_INTEGER) {
        if (value->flag || value->bytes.length == 0) { return false; }
        size_t leading = value->little_endian ? value->bytes.length-1 : 0;
        return (value->bytes.length == 1 || value->bytes.data[leading] != 0)
            && !(value->negative && value->bytes.length == 1 && value->bytes.data[0] == 0);
    }
    return (value->kind == AMA_DEBUG_TEXT || value->kind == AMA_DEBUG_BYTES)
        && !value->flag && !value->negative && !value->little_endian;
}
/** A successful reader emits exactly one scalar value. Observer stop never causes another observer call. */
static bool debug_native_emit(void *context, const AmaDebugNativeValue *value) {
    DebugNativeVisit *visitor = context;
    if (visitor->stopped || visitor->status.code != AMA_OK) { return false; }
    if (visitor->emitted || !debug_native_shape(value)) {
        visitor->status = ama_status_make(AMA_INVALID_STATE,NULL,0);
        return false;
    }
    visitor->emitted = true;
    visitor->stopped = !visitor->visit(visitor->context,value);
    return !visitor->stopped;
}
/** Missing adapters return a technical inspection result without invoking hooks or changing program/refusal status. */
AmaStatus ama_debug_native(AmaDebugSession *session, AmaContextReference reference, AmaDebugNativeVisitor visit, void *context) {
    if (visit == NULL) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    const AmaProgramType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_debug_resolve(session,reference,&type,&cell);
    if (status.code != AMA_OK) { return status; }
    status = ama_instance_access(&cell->metadata,AMA_ACCESS_READ);
    if (status.code != AMA_OK) { return status; }
    const AmaDebugNativeReader *reader = NULL;
    for (size_t index = 0; index < session->reader_count; index++) {
        if (session->readers[index].type_id == reference.local.type_id) { reader = &session->readers[index]; break; }
    }
    if (reader == NULL) { return ama_status_make(AMA_INVALID_STATE,"No registered native debug view",31); }
    status = ama_debug_visit_begin(session);
    if (status.code != AMA_OK) { return status; }
    DebugNativeVisit observer = {.visit=visit,.context=context,.status=ama_status_success()};
    const unsigned char *storage = ama_pool_payload(&type->object->pool,cell);
    status = ama_status_checked(reader->read(storage+type->object->native_offset,debug_native_emit,&observer));
    ama_debug_visit_end(session);
    if (observer.status.code != AMA_OK) { return observer.status; }
    if (status.code == AMA_OK && !observer.emitted) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }
    return status;
}
