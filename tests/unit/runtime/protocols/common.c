/**
 * @file
 * @brief Own injected raw protocol outcomes over real counted context resources.
 *
 * Reuse immutable context schemas. Each wrapper constructs/releases a real
 * address-sensitive payload before returning its explicitly selected transport.
 */
#ifndef AMA_PROTOCOL_TEST_COMMON_C
#define AMA_PROTOCOL_TEST_COMMON_C
#include "../contexts/common.c"
#include "runtime/lifecycle/native.h"
#include "runtime/program/calls/call.h"

static AmaStatus protocol_status;
static size_t protocol_defaults, protocol_literals, protocol_clones, protocol_sizes;
static size_t protocol_self_releases;

/** Reset independent context ownership and protocol observations for one case. */
static void setup_protocols(void) {
    setup_contexts();
    protocol_status = ama_status_make(AMA_OK, NULL, 1);
    protocol_defaults = protocol_literals = protocol_clones = protocol_sizes = 0;
    protocol_self_releases = 0;
}

/** A hook returning nonzero cleans its own native acquisition, even with bad message shape.
 * A zero-code payload remains live for the runtime's subsequently credited unwind.
 */
static AmaStatus protocol_outcome(void *payload, AmaStatus constructed_status) {
    if (constructed_status.code != AMA_OK) { return constructed_status; }
    if (protocol_status.code != AMA_OK) {
        NativeValue *value = payload;
        test_release(NULL, value->resource);
        value->resource = NULL;
        protocol_self_releases++;
    }
    return protocol_status;
}

/** Construct one fresh default payload, then return the selected raw transport. */
static AmaStatus protocol_initialize(void *payload) {
    protocol_defaults++;
    return protocol_outcome(payload, initialize_payload(payload));
}

/** Construct once through the literal callback; the registered default callback is untouched. */
static AmaStatus protocol_literal(void *payload, AmaByteView bytes) {
    protocol_literals++;
    return protocol_outcome(payload, literal_payload(payload, bytes));
}

/** Construct independent native copy storage before exposing its selected completion record. */
static AmaStatus protocol_clone(void *destination, const void *source) {
    protocol_clones++;
    return protocol_outcome(destination, clone_payload(destination, source));
}

/** Construct directly from internal host metadata; no literal parser or registered default runs. */
static AmaStatus protocol_from_size(void *payload, size_t value) {
    protocol_sizes++;
    AmaStatus status = initialize_payload(payload);
    if (status.code == AMA_OK) { ((NativeValue *)payload)->number = value; }
    return protocol_outcome(payload, status);
}

/** Release the real resource exactly once, then supply a deliberately malformed success. */
static AmaStatus protocol_destroy(void *payload) {
    AmaStatus status = destroy_payload(payload);
    return status.code == AMA_OK ? ama_status_make(AMA_OK, NULL, 1) : status;
}

/** Release the resource and report a distinct later cleanup failure. */
static AmaStatus protocol_destroy_io(void *payload) {
    AmaStatus status = destroy_payload(payload);
    return status.code == AMA_OK ? ama_status_make(AMA_IO_FAILURE, NULL, 0) : status;
}

/** Write a conversion output even on failure so the caller must protect its own destination. */
static AmaStatus protocol_condition(const void *payload, bool *result) {
    *result = ((const NativeValue *)payload)->number != 0;
    return protocol_status;
}

/** Retire the failed context, verify its primary status and finish the empty registry owner. */
static void finish_protocol_failure(uint64_t identity, uint32_t expected) {
    AmaStatus status = ama_context_dispose(&registry, identity);
    assert(status.code == expected && live_allocations == 0);
    finish_contexts();
}
#endif
