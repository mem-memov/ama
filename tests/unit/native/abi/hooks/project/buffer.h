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
} AMA_NATIVE(FixtureBuffer);

typedef struct {
    size_t value;
} AMA_NATIVE(FixtureCount);

typedef struct {
    const AMA_NATIVE(FixtureBuffer) *value;
} AMA_NATIVE(FixtureInputs);

typedef struct {
    AMA_NATIVE(FixtureCount) *length;
    AmaStatus *refusal;
} AMA_NATIVE(FixtureOutputs);

AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(FixtureBuffer) *destination);
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(FixtureBuffer) *state);
AmaStatus AMA_NATIVE(from_literal)(AMA_NATIVE(FixtureBuffer) *destination, AmaByteView literal);
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(FixtureBuffer) *destination, const AMA_NATIVE(FixtureBuffer) *source);
AmaStatus AMA_NATIVE(to_condition)(const AMA_NATIVE(FixtureBuffer) *state, bool *result);
AmaStatus AMA_NATIVE(to_index)(const AMA_NATIVE(FixtureBuffer) *state, size_t *result);

AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(FixtureBuffer) *state, const AMA_NATIVE(FixtureInputs) *parameters,
    AMA_NATIVE(FixtureOutputs) *results, const AmaNativeServices *services, AmaStatus *status) {
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
