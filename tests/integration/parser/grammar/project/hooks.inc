/**
 * @file
 * @brief Instantiate equal local Unicode/native protocol names under distinct caller namespaces.
 *
 * This C include borrows its enclosing fixture declarations and namespace.
 * The enclosing assertion suite compiles or compares its complete behavior.
 */
/**
 * Initialize the namespaced length payload with zero without allocating any native resource.
 */
static AmaStatus AMA_NATIVE(初始化)(AMA_NATIVE(NativeValue) *destination) {
    destination->length = 0;
    return ama_status_success();
}

/**
 * Reset the namespaced scalar payload; no owned resource or borrowed input survives cleanup.
 */
static AmaStatus AMA_NATIVE(destroy$value)(AMA_NATIVE(NativeValue) *state) {
    state->length = 0;
    return ama_status_success();
}

/**
 * Store only the exact literal byte length, retaining no literal pointer; this tests Unicode/UCN
 * helper identity rather than a library text format.
 */
static AmaStatus AMA_NATIVE(文)(AMA_NATIVE(NativeValue) *destination, AmaByteView literal) {
    destination->length = literal.length;
    return ama_status_success();
}

/**
 * Copy scalar payload data into independent destination State under the current module
 * namespace.
 */
static AmaStatus AMA_NATIVE(clone_value)(AMA_NATIVE(NativeValue) *destination, const AMA_NATIVE(NativeValue) *source) {
    *destination = *source;
    return ama_status_success();
}

/**
 * Read the scalar length and publish whether it is nonzero; never mutate native State.
 */
static AmaStatus AMA_NATIVE(condition_value)(const AMA_NATIVE(NativeValue) *state, bool *result) {
    *result = state->length != 0;
    return ama_status_success();
}

/**
 * Read the scalar length through the explicit size_t index signature without changing State.
 */
static AmaStatus AMA_NATIVE(index_value)(const AMA_NATIVE(NativeValue) *state, size_t *result) {
    *result = state->length;
    return ama_status_success();
}
