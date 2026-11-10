#include "echo.h"
#include "buffer.h"
#include "ama_types.h"

static AmaStatus initialize(const AmaType* type, void* destination) {
    (void)type;
    *(EchoState*)destination = (EchoState){0};
    return AMA_STATUS_OK;
}
static AmaStatus copy(const AmaType* type, void* destination, const void* source) {
    (void)type;
    *(EchoState*)destination = *(const EchoState*)source;
    return AMA_STATUS_OK;
}
static void move(const AmaType* type, void* destination, void* source) {
    (void)copy(type, destination, source);
}
static void destroy(const AmaType* type, void* state) {
    (void)type;
    (void)state;
}
const AmaType echo_descriptor = {
    .abi_version = AMA_VALUE_ABI_VERSION,
    .identity = UINT64_C(0xe957b3907961d9c0),
    .module_path = "native/echo",
    .size = sizeof(EchoState),
    .alignment = _Alignof(EchoState),
    .initialize = initialize,
    .initialize_copy = copy,
    .move = move,
    .destroy = destroy
};

AmaStatus ama_typed_native_e957b3907961d9c0(EchoState* state, const TestBuffer* input, TestBuffer* output) {
    state->executions++;
    AmaStatus status = ama_value_replace((AmaValue){&native_descriptor, output},
        (AmaValue){&native_descriptor, (void*)input});
    if (status != AMA_STATUS_OK) return status;
    return state->fail ? AMA_STATUS_LITERAL_INVALID : AMA_STATUS_OK;
}
