/** @file
 * @brief Own actual typed integer inputs, list service frames and clone resources.
 */
#ifndef AMA_LIST_INSERTION_TEST_COMMON
#define AMA_LIST_INSERTION_TEST_COMMON
#include "../list-service/common.c"
#include "runtime/program/lists/insertion/insertion.h"

/** Test integer representation is consumed only by its declared native protocols. */
typedef struct { size_t number; bool negative; void *resource; const void *address; } InsertionInteger;
static AmaObjectType integer_type;
static AmaObjectType *insertion_types[] = {&result_type, &owner_type, &integer_type};
static AmaProgramType insertion_program_types[3];
static size_t integer_defaults, integer_literals, integer_clones, integer_drops, index_calls, element_clones;
static AmaStatus index_status;

/** Allocate one integer payload directly into its final native address. */
static AmaStatus integer_construct(void *storage, size_t number, bool negative) {
    InsertionInteger *value = storage;
    *value = (InsertionInteger){.number = number, .negative = negative, .address = storage};
    value->resource = test_allocate(NULL, 1);
    return value->resource == NULL ? ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0) : ama_status_success();
}
/** Default is deliberately observable; omitted method positions must use literal zero. */
static AmaStatus integer_default(void *storage) {
    integer_defaults++;
    return integer_construct(storage, 999, false);
}
/** Parse every nonnegative host-size value exactly and retain negative sign for refusal. */
static AmaStatus integer_literal(void *storage, AmaByteView bytes) {
    integer_literals++;
    bool negative = bytes.length != 0 && bytes.data[0] == '-';
    size_t first = negative ? 1 : 0, value = 0;
    if (first == bytes.length) { return ama_status_make(AMA_INVALID_LITERAL, NULL, 0); }
    for (size_t index = first; index < bytes.length; index++) {
        unsigned char byte = bytes.data[index];
        if (byte < '0' || byte > '9' || value > (SIZE_MAX - (byte - '0')) / 10) {
            return ama_status_make(AMA_INVALID_LITERAL, NULL, 0);
        }
        value = value * 10 + (byte - '0');
    }
    return integer_construct(storage, value, negative && value != 0);
}
/** Ordinary parameter copy clones its resource and fixes its actual address. */
static AmaStatus integer_clone(void *storage, const void *source) {
    const InsertionInteger *value = source;
    assert(value->address == source && value->resource != NULL);
    integer_clones++;
    return integer_construct(storage, value->number, value->negative);
}
/** Destroy only successful integer credits, regardless of later method refusal. */
static AmaStatus integer_destroy(void *storage) {
    InsertionInteger *value = storage;
    assert(value->address == storage && value->resource != NULL);
    test_release(NULL, value->resource);
    value->resource = NULL;
    integer_drops++;
    return ama_status_success();
}
/** Write even on forced failure; callers must never consume a failed conversion. */
static AmaStatus integer_index(const void *storage, size_t *position) {
    const InsertionInteger *value = storage;
    assert(value->address == storage && value->resource != NULL);
    index_calls++;
    *position = value->number;
    if (index_status.code != AMA_OK || !ama_status_has_valid_message_view(index_status)) { return index_status; }
    return value->negative ? ama_status_make(AMA_INVALID_INDEX, NULL, 0) : ama_status_success();
}
/** Clone actual element bytes into an independent resource without a default credit. */
static AmaStatus insertion_element_clone(void *storage, const void *source) {
    const ServiceResult *original = source;
    ServiceResult *value = storage;
    assert(original->address == source && original->resource != NULL);
    *value = (ServiceResult){.address = storage, .resource = test_allocate(NULL, 8)};
    if (value->resource == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    memcpy(value->resource, original->resource, 8);
    element_clones++;
    return ama_status_success();
}
/** A legitimate native constructor may return Busy technically after cleaning its prefix. */
static AmaStatus insertion_busy_constructor(void *storage) {
    AmaStatus status = result_default(storage);
    if (status.code != AMA_OK) { return status; }
    ServiceResult *value = storage;
    test_release(NULL, value->resource);
    value->resource = NULL;
    return ama_status_make(AMA_BUSY, NULL, 0);
}
/** Fail a native clone after cleaning its own allocated prefix, without credit. */
static AmaStatus insertion_busy_clone(void *storage, const void *source) {
    const ServiceResult *original = source;
    assert(original->address == source && original->resource != NULL);
    ServiceResult *value = storage;
    *value = (ServiceResult){.address = storage, .resource = test_allocate(NULL, 8)};
    if (value->resource == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    test_release(NULL, value->resource);
    value->resource = NULL;
    return ama_status_make(AMA_BUSY, NULL, 0);
}
/** Extend the real list fixture with an independently typed native index module. */
static void setup_insertion(void) {
    setup_service();
    integer_defaults = integer_literals = integer_clones = integer_drops = index_calls = element_clones = 0;
    index_status = ama_status_success();
    AmaStatus status = ama_object_type_init(&integer_type, sources.allocator, 3, sizeof(InsertionInteger),
        2, UINT64_MAX, NULL, 0, 0, (AmaPayloadHooks){integer_default, integer_literal, integer_clone, integer_destroy});
    assert(status.code == AMA_OK);
    result_type.hooks.clone = insertion_element_clone;
    world.types = insertion_types;
    world.type_count = 3;
    insertion_program_types[0] = (AmaProgramType){.object = &result_type};
    insertion_program_types[1] = (AmaProgramType){.object = &owner_type};
    insertion_program_types[2] = (AmaProgramType){.object = &integer_type, .index = integer_index};
    program.types = insertion_program_types;
    program.type_count = 3;
}
/** All typed input roots must be retired before tearing down their pool. */
static void finish_insertion(void) {
    assert(integer_type.pool.live_count == 0);
    ama_pool_dispose(&integer_type.pool);
    finish_service();
}
/** Open one expected declared list under the current caller's own protection. */
static AmaProgramListService *open_insertion(size_t field) {
    AmaProgramListService *service = &owner_value(0)->services[0];
    bool refused = true;
    AmaStatus status = ama_program_list_open(&program, owners[0], owners[0], field, service, &refused);
    assert(status.code == AMA_OK && !refused);
    return service;
}
/** Finish one method and preserve the parent grant for another sibling. */
static void end_insertion(AmaProgramListMethod *method) {
    AmaStatus status = ama_program_list_method_end(method);
    assert(status.code == AMA_OK);
}
/** Release the service before public owner teardown. */
static void close_insertion(AmaProgramListService *service) {
    AmaStatus status = ama_program_list_close(service);
    assert(status.code == AMA_OK);
}
#endif
