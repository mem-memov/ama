#ifndef FIXTURE_BUFFER_H
#define FIXTURE_BUFFER_H

#include <stdbool.h>
#include "native/abi/bytes.h"
#include "native/abi/services.h"
#include "native/toolchain/gcc.h"

typedef struct {
    unsigned char *data;
    size_t length;
    size_t entry_count;
    uint32_t conversion_error;
    uint32_t destroy_error;
    uint32_t clone_error;
} FixtureBuffer;

typedef struct {
    size_t value;
} FixtureCount;

typedef struct {
    const FixtureBuffer *value;
} FixtureInputs;

typedef struct {
    FixtureCount *length;
    AmaStatus *refusal;
} FixtureOutputs;

AmaStatus fixture_initialize(FixtureBuffer *destination);
AmaStatus fixture_destroy(FixtureBuffer *state);
AmaStatus fixture_from_literal(FixtureBuffer *destination, AmaByteView literal);
AmaStatus fixture_clone(FixtureBuffer *destination, const FixtureBuffer *source);
AmaStatus fixture_to_condition(const FixtureBuffer *state, bool *result);
AmaStatus fixture_to_index(const FixtureBuffer *state, size_t *result);

AMA_NATIVE_ENTRY void fixture_entry(FixtureBuffer *state, const FixtureInputs *parameters,
    FixtureOutputs *results, const AmaNativeServices *services, AmaStatus *status) {
    if (state->entry_count == SIZE_MAX) {
        *status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0);
        return;
    }
    state->entry_count++;
    results->length->value = parameters->value->length;
    *results->refusal = services->caller_refusal;
    *status = ama_status_success();
}

#endif
