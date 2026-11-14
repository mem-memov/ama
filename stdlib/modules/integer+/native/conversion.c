/**
 * @file
 * @brief Inspect integer state through explicit condition and checked index protocols.
 * [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03)
 * defines fresh construction, full host-size roundtrips and checked conversions.
 */

/** Read mathematical nonzero state without entering the module or changing its payload. */
AmaStatus AMA_NATIVE(to_condition)(const AMA_NATIVE(State) *state, bool *result) {
    if (!state->live) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    *result = state->count != 1 || state->limbs[0] != 0;
    return ama_status_success();
}

/** Fold unsigned base-256 digits only while the next multiply/add fits size_t.
 * Negative or overflowing values return exactly InvalidIndex and preserve output.
 */
AmaStatus AMA_NATIVE(to_index)(const AMA_NATIVE(State) *state, size_t *result) {
    if (!state->live) { return ama_status_make(AMA_INVALID_STATE, NULL, 0); }
    if (state->negative) { return ama_status_make(AMA_INVALID_INDEX, NULL, 0); }
    size_t value = 0;
    for (size_t index = state->count; index != 0; index--) {
        unsigned limb = state->limbs[index - 1];
        if (value > (SIZE_MAX - limb) / 256) { return ama_status_make(AMA_INVALID_INDEX, NULL, 0); }
        value = value * 256 + limb;
    }
    *result = value;
    return ama_status_success();
}
