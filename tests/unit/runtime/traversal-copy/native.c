/** @file
 * @brief Own address-sensitive clone resources for continuation failure-prefix checks.
 */
/** Successful native construction grants one counted resource at its final address. */
typedef struct { void *resource; const void *address; } CopyNative;
static AmaObjectType native_type;
static AmaObjectType *native_types[] = {&owner_type, &native_type};
static size_t native_clones, native_drops, native_fail_at;

/** A failed allocator owns no resource and creates no destruction credit. */
static AmaStatus copy_native_default(void *storage) {
    CopyNative *value = storage;
    *value = (CopyNative){.resource = test_allocate(NULL, 1), .address = storage};
    return value->resource == NULL ? ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0) : ama_status_success();
}
/** Failed clones release their own partial allocation; earlier successful clones
 * remain the enclosing object's initialized-prefix cleanup responsibility.
 */
static AmaStatus copy_native_clone(void *storage, const void *source) {
    const CopyNative *original = source;
    assert(original->address == source && original->resource != NULL);
    native_clones++;
    AmaStatus status = copy_native_default(storage);
    if (status.code == AMA_OK && native_clones == native_fail_at) {
        CopyNative *value = storage;
        test_release(NULL, value->resource);
        value->resource = NULL;
        return ama_status_make(AMA_BUSY, NULL, 0);
    }
    return status;
}
/** Every successful prefix credit is consumed once despite later copy failure. */
static AmaStatus copy_native_destroy(void *storage) {
    CopyNative *value = storage;
    assert(value->address == storage && value->resource != NULL);
    test_release(NULL, value->resource);
    value->resource = NULL;
    native_drops++;
    return ama_status_success();
}
/** Specialize the still-empty owned field to actual independent native resources. */
static void setup_native_copy(void) {
    setup_copy(true, false);
    static const AmaObjectField fields[] = {
        {AMA_FIELD_REFERENCE_LIST, 1, offsetof(OriginOwner, items)},
        {AMA_FIELD_REFERENCE, 1, offsetof(OriginOwner, reused)},
        {AMA_FIELD_REFERENCE, 1, offsetof(OriginOwner, independent)},
        {AMA_FIELD_REFERENCE_LIST, 1, offsetof(OriginOwner, other)},
        {AMA_FIELD_OWNING_LIST, 2, offsetof(OriginOwner, owned)}
    };
    owner_type.fields = fields;
    checked(ama_object_type_init(&native_type, sources.allocator, 2, sizeof(CopyNative),
        2, UINT64_MAX, NULL, 0, 0, (AmaPayloadHooks){copy_native_default, NULL, copy_native_clone, copy_native_destroy}));
    world.types = native_types;
    world.type_count = 2;
    native_clones = native_drops = native_fail_at = 0;
    AmaListStorage storage = {&world, owners[0], &owner_value(0)->owned, 2, true};
    checked(ama_list_storage_insert(&storage, 0, (AmaHandle){0}));
    checked(ama_list_storage_insert(&storage, 1, (AmaHandle){0}));
}
/** Release source native children before the shared fixture disposes its allocator. */
static void finish_native_copy(void) {
    AmaListStorage storage = {&world, owners[0], &owner_value(0)->owned, 2, true};
    checked(ama_list_storage_remove(&storage, 0, storage.list->count));
    ama_pool_dispose(&native_type.pool);
    finish_copy();
}
