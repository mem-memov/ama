/** @file
 * @brief Own real pool-backed list frames and counted ephemeral native resources.
 */
#ifndef AMA_LIST_SERVICE_TEST_COMMON
#define AMA_LIST_SERVICE_TEST_COMMON
#include "../../source/common.c"
#include "runtime/program/lists/service.h"
#include "runtime/program/calls/call.h"
#include "runtime/program/values/selection/internal.h"
#include <stddef.h>

/** Declared fields and transient lexical frames occupy disjoint storage. */
typedef struct {
    AmaHandleList items;
    AmaHandleList other;
    AmaHandle parent;
    unsigned char payload;
    AmaProgramListService services[2];
    AmaProgramListMethod methods[2];
} ServiceOwner;
/** One address-sensitive resource proves selected construction and exact cleanup. */
typedef struct { void *resource; const void *address; } ServiceResult;
static AmaObjectType owner_type, result_type;
static AmaObjectType *service_types[] = {&result_type, &owner_type};
static AmaWorld world;
static AmaProgram program;
static AmaHandle owners[3];
static size_t defaults, literals, drops;
static bool constructor_failure, destructor_failure, check_callback;

/** Allocate exactly one native resource; a failed constructor cleans its own prefix. */
static AmaStatus result_default(void *storage) {
    ServiceResult *value = storage;
    value->address = value;
    defaults++;
    value->resource = test_allocate(NULL, 8);
    if (value->resource == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    if (constructor_failure) {
        test_release(NULL, value->resource);
        value->resource = NULL;
        return ama_status_make(AMA_NATIVE_FAILURE, NULL, 0);
    }
    return ama_status_success();
}

/** Select literal construction directly; no prior default constructor runs. */
static AmaStatus result_literal(void *storage, AmaByteView literal) {
    assert(literal.length == 1 && literal.data[0] == 'x');
    AmaStatus status = result_default(storage);
    defaults--;
    literals++;
    return status;
}

/** Borrow a known live frame owner without retaining unstable collection arrays. */
static ServiceOwner *owner_value(size_t index) {
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(&program, owners[index], &cell);
    assert(status.code == AMA_OK);
    return ama_pool_payload(&owner_type.pool, cell);
}

/** Require normal checked lookup before changing test-controlled activity. */
static void owner_activity(size_t index, AmaInstanceActivity activity) {
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(&program, owners[index], &cell);
    assert(status.code == AMA_OK);
    cell->metadata.service.activity = activity;
}

/** Consume resources even on failure. A callback must still see the parent list
 * busy and cannot close it while result cleanup is in progress.
 */
static AmaStatus result_destroy(void *storage) {
    ServiceResult *value = storage;
    assert(value->address == value && value->resource != NULL);
    if (check_callback) {
        AmaProgramValue selected = {.type_id = 99};
        const AmaProgramAccess access = {.field = 0};
        AmaStatus status = ama_program_select(&program, owners[0], &access, false, &selected);
        assert(status.code == AMA_BUSY && selected.type_id == 99);
        status = ama_program_list_close(&owner_value(0)->services[0]);
        assert(status.code == AMA_INVALID_STATE && owner_value(0)->items.acquisition != NULL);
    }
    test_release(NULL, value->resource);
    value->resource = NULL;
    drops++;
    return destructor_failure ? ama_status_make(AMA_IO_FAILURE, NULL, 0) : ama_status_success();
}

/** Construct explicit owners and mark only the initiating caller Running. */
static void setup_service(void) {
    setup_test();
    defaults = literals = drops = 0;
    constructor_failure = destructor_failure = check_callback = false;
    AmaStatus status = ama_object_type_init(&result_type, sources.allocator, 1,
        sizeof(ServiceResult), 2, UINT64_MAX, NULL, 0, 0,
        (AmaPayloadHooks){result_default, result_literal, NULL, result_destroy});
    assert(status.code == AMA_OK);
    static const AmaObjectField fields[] = {
        {AMA_FIELD_REFERENCE_LIST, 1, offsetof(ServiceOwner, items)},
        {AMA_FIELD_OWNING_LIST, 1, offsetof(ServiceOwner, other)},
        {AMA_FIELD_REFERENCE, 2, offsetof(ServiceOwner, parent)}
    };
    status = ama_object_type_init(&owner_type, sources.allocator, 2, sizeof(ServiceOwner),
        2, UINT64_MAX, fields, 3, offsetof(ServiceOwner, payload), (AmaPayloadHooks){0});
    assert(status.code == AMA_OK);
    world = (AmaWorld){.types = service_types, .type_count = 2, .allocator = sources.allocator};
    program = (AmaProgram){.world = &world};
    for (size_t index = 0; index < 3; index++) {
        status = ama_object_create(&world, 2, NULL, &owners[index]);
        assert(status.code == AMA_OK);
    }
    owner_value(0)->parent = owner_value(2)->parent = owners[1];
    program.current = owners[0];
    owner_activity(0, AMA_INSTANCE_RUNNING);
}

/** Release real roots after all grants and temporary result credits are gone. */
static void finish_service(void) {
    for (size_t index = 0; index < 3; index++) {
        ServiceOwner *owner = owner_value(index);
        assert(owner->items.acquisition == NULL && owner->other.acquisition == NULL);
        owner_activity(index, AMA_INSTANCE_READY);
    }
    for (size_t index = 3; index != 0; index--) {
        AmaStatus status = ama_object_destroy(&world, owners[index - 1]);
        assert(status.code == AMA_OK);
    }
    ama_pool_dispose(&owner_type.pool);
    ama_pool_dispose(&result_type.pool);
    destroy_test();
}
#endif
