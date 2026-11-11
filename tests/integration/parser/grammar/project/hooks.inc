static AmaStatus AMA_NATIVE(初始化)(AMA_NATIVE(NativeValue) *destination) {
    destination->length = 0;
    return ama_status_success();
}

static AmaStatus AMA_NATIVE(destroy$value)(AMA_NATIVE(NativeValue) *state) {
    state->length = 0;
    return ama_status_success();
}

static AmaStatus AMA_NATIVE(文)(AMA_NATIVE(NativeValue) *destination, AmaByteView literal) {
    destination->length = literal.length;
    return ama_status_success();
}

static AmaStatus AMA_NATIVE(clone_value)(AMA_NATIVE(NativeValue) *destination, const AMA_NATIVE(NativeValue) *source) {
    *destination = *source;
    return ama_status_success();
}

static AmaStatus AMA_NATIVE(condition_value)(const AMA_NATIVE(NativeValue) *state, bool *result) {
    *result = state->length != 0;
    return ama_status_success();
}

static AmaStatus AMA_NATIVE(index_value)(const AMA_NATIVE(NativeValue) *state, size_t *result) {
    *result = state->length;
    return ama_status_success();
}
