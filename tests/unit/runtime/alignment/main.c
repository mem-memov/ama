#include "../../source/common.c"
#include "runtime/objects/objects.h"

typedef struct { _Alignas(128) void *address; size_t number; } AlignedValue;
static size_t destroyed;

static AmaStatus initialize(void *payload) {
    AlignedValue *value = payload;
    assert((uintptr_t)payload % _Alignof(AlignedValue) == 0);
    value->address = value;
    value->number = 17;
    return ama_status_success();
}

static AmaStatus clone(void *destination, const void *source) {
    AmaStatus status = initialize(destination);
    ((AlignedValue *)destination)->number = ((const AlignedValue *)source)->number;
    return status;
}

static AmaStatus finalize(void *payload) {
    AlignedValue *value = payload;
    assert(value->address == value && value->number == 17);
    destroyed++;
    return ama_status_success();
}

static void it_preserves_extended_c_alignment_through_growth_replacement_and_reclamation(void) {
    setup_test();
    AmaObjectType type;
    AmaPayloadHooks hooks = {.initialize = initialize, .clone = clone, .destroy = finalize};
    AmaStatus status = ama_object_type_init_aligned(&type, sources.allocator, 1,
        sizeof(AlignedValue), _Alignof(AlignedValue), 2, 2, NULL, 0, 0, hooks);
    assert(status.code == AMA_OK);
    AmaObjectType *registry[] = {&type};
    AmaWorld world = {.types = registry, .type_count = 1, .allocator = sources.allocator};
    AmaHandle handles[8];
    for (size_t index = 0; index < 8; index++) {
        status = ama_object_create(&world, 1, NULL, &handles[index]);
        assert(status.code == AMA_OK);
    }
    for (size_t index = 1; index < 8; index++) {
        status = ama_object_replace(&world, handles[index], handles[0], false);
        assert(status.code == AMA_OK);
    }
    for (size_t index = 0; index < 8; index++) {
        status = ama_object_destroy(&world, handles[index]);
        assert(status.code == AMA_OK);
    }
    size_t released;
    status = ama_pool_reclaim(&type.pool, &released);
    assert(status.code == AMA_OK && released != 0 && destroyed == 15);
    ama_pool_dispose(&type.pool);
    destroy_test();
}

static void it_rejects_nonrepresentable_alignment_and_size_arithmetic(void) {
    AmaPool pool;
    AmaStatus status = ama_pool_init_aligned(&pool, source_system_allocator(), 1, 1, 3, 2, 2);
    assert(status.code == AMA_INVALID_STATE);
    status = ama_pool_init_aligned(&pool, source_system_allocator(), 1, SIZE_MAX, 128, 2, 2);
    assert(status.code == AMA_LIMIT_EXCEEDED);
}

void run(void) {
    it_preserves_extended_c_alignment_through_growth_replacement_and_reclamation();
    it_rejects_nonrepresentable_alignment_and_size_arithmetic();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
