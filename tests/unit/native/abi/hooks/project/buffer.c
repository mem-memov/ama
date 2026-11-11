#include "buffer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

AmaStatus fixture_initialize(FixtureBuffer *destination) {
    *destination = (FixtureBuffer) {0};
    return ama_status_success();
}

AmaStatus fixture_destroy(FixtureBuffer *state) {
    uint32_t error = state->destroy_error;
    free(state->data);
    *state = (FixtureBuffer) {0};
    return ama_status_make(error, NULL, 0);
}

AmaStatus fixture_from_literal(FixtureBuffer *destination, AmaByteView literal) {
    *destination = (FixtureBuffer) {0};
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

AmaStatus fixture_clone(FixtureBuffer *destination, const FixtureBuffer *source) {
    if (source->clone_error != AMA_OK) {
        return ama_status_make(source->clone_error, NULL, 0);
    }
    AmaByteView literal = {source->data, source->length};
    AmaStatus status = fixture_from_literal(destination, literal);
    if (ama_status_is_success(status)) {
        destination->entry_count = source->entry_count;
        destination->conversion_error = source->conversion_error;
        destination->destroy_error = source->destroy_error;
    }
    return status;
}

AmaStatus fixture_to_condition(const FixtureBuffer *state, bool *result) {
    if (state->conversion_error != AMA_OK) {
        return ama_status_make(state->conversion_error, NULL, 0);
    }
    *result = state->length != 0;
    return ama_status_success();
}

AmaStatus fixture_to_index(const FixtureBuffer *state, size_t *result) {
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
