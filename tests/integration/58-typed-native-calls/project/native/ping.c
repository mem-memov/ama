#include "ping.h"
#include "ama_types.h"
static AmaStatus initialize(const AmaType* type, void* destination) {
    (void)type;
    *(size_t*)destination = 0;
    return AMA_STATUS_OK;
}
static AmaStatus copy(const AmaType* type, void* destination, const void* source) {
    (void)type;
    *(size_t*)destination = *(const size_t*)source;
    return AMA_STATUS_OK;
}
static void move(const AmaType* type, void* destination, void* source) {
    (void)copy(type, destination, source);
}
static void destroy(const AmaType* type, void* state) {
    (void)type;
    (void)state;
}
const AmaType ama_type_ad4c8e009dd42c75 = {
    .abi_version = AMA_VALUE_ABI_VERSION,
    .identity = UINT64_C(0xad4c8e009dd42c75),
    .module_path = "native/ping",
    .size = sizeof(size_t),
    .alignment = _Alignof(size_t),
    .initialize = initialize,
    .initialize_copy = copy,
    .move = move,
    .destroy = destroy
};
AmaStatus ama_typed_native_ad4c8e009dd42c75(size_t* executions) {
    (*executions)++;
    return AMA_STATUS_OK;
}
