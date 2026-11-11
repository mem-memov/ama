#include "native.h"

static AmaStatus 初始化(NativeValue *destination) {
    destination->length = 0;
    return ama_status_success();
}

static AmaStatus destroy$value(NativeValue *state) {
    state->length = 0;
    return ama_status_success();
}

static AmaStatus 文(NativeValue *destination, AmaByteView literal) {
    destination->length = literal.length;
    return ama_status_success();
}

static AmaStatus clone_value(NativeValue *destination, const NativeValue *source) {
    *destination = *source;
    return ama_status_success();
}

static AmaStatus condition_value(const NativeValue *state, bool *result) {
    *result = state->length != 0;
    return ama_status_success();
}

static AmaStatus index_value(const NativeValue *state, size_t *result) {
    *result = state->length;
    return ama_status_success();
}
