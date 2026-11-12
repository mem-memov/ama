/**
 * @file
 * @brief Implement owned byte-buffer lifecycle and explicitly injected native conversion failures.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "buffer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/**
 * Initialize one fresh native State with its success default; no prior live resource may occupy
 * destination.
 */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(FixtureBuffer) *destination) {
    *destination = (AMA_NATIVE(FixtureBuffer)) {0};
    return ama_status_success();
}

/**
 * Release only successfully constructed native State. Retained scalar modules own no resource;
 * fixture-specific injected failures are returned after cleanup.
 */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(FixtureBuffer) *state) {
    uint32_t error = state->destroy_error;
    free(state->data);
    *state = (AMA_NATIVE(FixtureBuffer)) {0};
    return ama_status_make(error, NULL, 0);
}

/**
 * Allocate independent owned bytes from the exact bounded literal; initialize failed
 * destinations safely and retain no borrowed literal pointer.
 */
AmaStatus AMA_NATIVE(from_literal)(AMA_NATIVE(FixtureBuffer) *destination, AmaByteView literal) {
    *destination = (AMA_NATIVE(FixtureBuffer)) {0};
    if (literal.length > PTRDIFF_MAX) {
        return ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
    }
    if (literal.length != 0) {
        destination->data = malloc(literal.length);
        if (destination->data == NULL) {
            return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0);
        }
        memcpy(destination->data, literal.data, literal.length);
    }
    destination->length = literal.length;
    return ama_status_success();
}

/**
 * Copy source native data into fresh destination storage without copying runtime identity or
 * transient execution links; source stays unchanged.
 */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(FixtureBuffer) *destination, const AMA_NATIVE(FixtureBuffer) *source) {
    if (source->clone_error != AMA_OK) {
        return ama_status_make(source->clone_error, NULL, 0);
    }
    AmaByteView literal = {source->data, source->length};
    AmaStatus status = AMA_NATIVE(from_literal)(destination, literal);
    if (ama_status_is_success(status)) {
        destination->entry_count = source->entry_count;
        destination->conversion_error = source->conversion_error;
        destination->destroy_error = source->destroy_error;
    }
    return status;
}

/**
 * Read buffer length through the explicit condition hook; return the selected injected technical
 * failure without mutating native resources.
 */
AmaStatus AMA_NATIVE(to_condition)(const AMA_NATIVE(FixtureBuffer) *state, bool *result) {
    if (state->conversion_error != AMA_OK) {
        return ama_status_make(state->conversion_error, NULL, 0);
    }
    *result = state->length != 0;
    return ama_status_success();
}

/**
 * Read buffer length through the explicit index hook; preserve the distinct injected
 * invalid-index/technical status for caller classification.
 */
AmaStatus AMA_NATIVE(to_index)(const AMA_NATIVE(FixtureBuffer) *state, size_t *result) {
    if (state->conversion_error != AMA_OK) {
        return ama_status_make(state->conversion_error, NULL, 0);
    }
    if (state->length == 0) {
        return ama_status_make(AMA_INVALID_INDEX, NULL, 0);
    }
    size_t value = 0;
    for (size_t index = 0; index < state->length; index++) {
        unsigned char byte = state->data[index];
        if (byte < '0' || byte > '9' || value > (SIZE_MAX - (byte - '0')) / 10) {
            return ama_status_make(AMA_INVALID_INDEX, NULL, 0);
        }
        value = value * 10 + (byte - '0');
    }
    *result = value;
    return ama_status_success();
}
