#include "native.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

size_t live_resources;
size_t literal_attempts;
size_t fail_literal_at;
size_t copy_attempts;
size_t fail_copy_at;

static AmaStatus initialize_literal(const AmaType* type, void* destination,
    const unsigned char* bytes, size_t length) {
    (void)type;
    literal_attempts++;
    if (literal_attempts == fail_literal_at) return AMA_STATUS_LITERAL_INVALID;
    unsigned char* owned = malloc(length ? length : 1);
    if (!owned) return AMA_STATUS_MEMORY;
    if (length) memcpy(owned, bytes, length);
    *(TestBuffer*)destination = (TestBuffer){owned, length, destination};
    live_resources++;
    return AMA_STATUS_OK;
}

static AmaStatus initialize(const AmaType* type, void* destination) {
    return initialize_literal(type, destination, NULL, 0);
}

static AmaStatus initialize_copy(const AmaType* type, void* destination, const void* source) {
    copy_attempts++;
    if (copy_attempts == fail_copy_at) return AMA_STATUS_COPY_UNSUPPORTED;
    const TestBuffer* buffer = source;
    assert(buffer->home == buffer);
    return initialize_literal(type, destination, buffer->bytes, buffer->length);
}

static void move(const AmaType* type, void* destination, void* source) {
    (void)type;
    TestBuffer* original = source;
    assert(original->home == original);
    *(TestBuffer*)destination = *original;
    ((TestBuffer*)destination)->home = destination;
    *original = (TestBuffer){0};
}

static void destroy(const AmaType* type, void* state) {
    (void)type;
    TestBuffer* buffer = state;
    assert(buffer->home == buffer && live_resources);
    free(buffer->bytes);
    *buffer = (TestBuffer){0};
    live_resources--;
}

const AmaType native_descriptor = {
    .abi_version = AMA_VALUE_ABI_VERSION,
    .identity = TEST_NATIVE_IDENTITY,
    .module_path = "native/buffer",
    .size = sizeof(TestBuffer),
    .alignment = _Alignof(TestBuffer),
    .initialize = initialize,
    .initialize_literal = initialize_literal,
    .initialize_copy = initialize_copy,
    .move = move,
    .destroy = destroy
};
