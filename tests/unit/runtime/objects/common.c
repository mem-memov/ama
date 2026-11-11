#ifndef AMA_OBJECT_TEST_COMMON
#define AMA_OBJECT_TEST_COMMON
#include "../../source/common.c"
#include "runtime/objects/objects.h"

typedef struct { void *resource; size_t number; void *address; } NativeValue;
typedef struct { AmaHandle peer; AmaHandle root; NativeValue native; } LeafValue;
typedef struct {
    AmaHandle left;
    AmaHandle right;
    AmaHandleList branches;
    AmaHandle selected;
    AmaHandle self;
    AmaHandleList references;
    NativeValue native;
} RootValue;
static AmaObjectType leaf_type, root_type;
static AmaObjectType *types[] = {&leaf_type, &root_type};
static AmaWorld world;
static size_t constructed, cloned, destroyed, failed_constructor, failed_destructor, reports;
static size_t destruction_order[100];

static AmaStatus initialize_payload(void *address) {
    NativeValue *value = address;
    value->address = value;
    value->number = ++constructed;
    value->resource = test_allocate(NULL, 1);
    if (value->resource == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0); }
    if (constructed == failed_constructor) {
        test_release(NULL, value->resource);
        value->resource = NULL;
        return ama_status_make(AMA_INVALID_LITERAL, NULL, 0);
    }
    return ama_status_success();
}

static AmaStatus literal_payload(void *address, AmaByteView literal) {
    AmaStatus status = initialize_payload(address);
    if (status.code == AMA_OK && literal.length != 0) { ((NativeValue *)address)->number = literal.data[0]; }
    return status;
}

static AmaStatus clone_payload(void *address, const void *source) {
    cloned++;
    AmaStatus status = initialize_payload(address);
    if (status.code == AMA_OK) { ((NativeValue *)address)->number = ((const NativeValue *)source)->number; }
    return status;
}

static AmaStatus destroy_payload(void *address) {
    NativeValue *value = address;
    assert(value->resource != NULL && value->address == value && destroyed < 100);
    destruction_order[destroyed++] = value->number;
    test_release(NULL, value->resource);
    value->resource = NULL;
    return destroyed == failed_destructor ? ama_status_make(AMA_IO_FAILURE, NULL, 0) : ama_status_success();
}

static void report_status(void *context, AmaStatus status) {
    (void)context;
    assert(status.code != AMA_OK);
    reports++;
}

static void setup_objects(void) {
    setup_test();
    constructed = cloned = destroyed = failed_constructor = failed_destructor = reports = 0;
    static const AmaObjectField leaf_fields[] = {
        {AMA_FIELD_REFERENCE, 1, offsetof(LeafValue, peer)},
        {AMA_FIELD_REFERENCE, 2, offsetof(LeafValue, root)}
    };
    static const AmaObjectField root_fields[] = {
        {AMA_FIELD_OWNED, 1, offsetof(RootValue, left)},
        {AMA_FIELD_OWNED, 1, offsetof(RootValue, right)},
        {AMA_FIELD_OWNING_LIST, 2, offsetof(RootValue, branches)},
        {AMA_FIELD_REFERENCE, 1, offsetof(RootValue, selected)},
        {AMA_FIELD_REFERENCE, 2, offsetof(RootValue, self)},
        {AMA_FIELD_REFERENCE_LIST, 1, offsetof(RootValue, references)}
    };
    AmaPayloadHooks hooks = {initialize_payload, literal_payload, clone_payload, destroy_payload};
    AmaStatus status = ama_object_type_init(&leaf_type, sources.allocator, 1, sizeof(LeafValue), 2, 3,
        leaf_fields, 2, offsetof(LeafValue, native), hooks);
    assert(status.code == AMA_OK);
    status = ama_object_type_init(&root_type, sources.allocator, 2, sizeof(RootValue), 2, 3,
        root_fields, 6, offsetof(RootValue, native), hooks);
    assert(status.code == AMA_OK);
    world = (AmaWorld){.types = types, .type_count = 2, .allocator = sources.allocator, .report = report_status};
}

static void destroy_objects(void) {
    assert(leaf_type.pool.live_count == 0 && root_type.pool.live_count == 0);
    ama_pool_dispose(&leaf_type.pool);
    ama_pool_dispose(&root_type.pool);
    destroy_test();
}

static inline void *object_payload(AmaHandle handle) {
    AmaObjectType *type = ama_world_type(&world, handle.type_id);
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_pool_lookup(&type->pool, handle, false, &cell);
    assert(status.code == AMA_OK);
    return ama_pool_payload(&type->pool, cell);
}
#endif
