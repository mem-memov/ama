/** @file
 * @brief Borrow only aligned, bounded caller-owned lexical list frame storage.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) keeps these service
 * cells stable; [LIST-04](../../../../spec/language/lists.md#list-04) excludes
 * method activations from Ama instances and native payload ownership.
 */
#include "service.h"
#include "runtime/program/calls/call.h"

/** Validate generated frame extent before exposing a stable pool address.
 * Offsets are trusted generated descriptors, never a public foreign pointer API.
 */
static AmaStatus ama_list_frame(AmaProgram *program, AmaHandle caller, size_t offset,
    size_t size, size_t alignment, void **output) {
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(program, caller, &cell);
    if (status.code != AMA_OK) { return status; }
    AmaObjectType *type = ama_world_type(program->world, caller.type_id);
    if (offset % alignment != 0 || offset > type->pool.payload_size || size > type->pool.payload_size - offset) {
        return ama_status_make(AMA_INVALID_STATE, NULL, 0);
    }
    *output = (unsigned char *)ama_pool_payload(&type->pool, cell) + offset;
    return ama_status_success();
}

AmaStatus ama_program_list_service_frame(AmaProgram *program, AmaHandle caller,
    size_t offset, AmaProgramListService **output) {
    void *frame = NULL;
    AmaStatus status = ama_list_frame(program, caller, offset, sizeof(AmaProgramListService),
        _Alignof(AmaProgramListService), &frame);
    if (status.code == AMA_OK) { *output = frame; }
    return status;
}

AmaStatus ama_program_list_method_frame(AmaProgram *program, AmaHandle caller,
    size_t offset, AmaProgramListMethod **output) {
    void *frame = NULL;
    AmaStatus status = ama_list_frame(program, caller, offset, sizeof(AmaProgramListMethod),
        _Alignof(AmaProgramListMethod), &frame);
    if (status.code == AMA_OK) { *output = frame; }
    return status;
}
