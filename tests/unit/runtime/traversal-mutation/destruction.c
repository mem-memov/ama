/** @file
 * @brief Own native resources whose terminal deletion still publishes cursor correction.
 */
/** One successful native credit has an actual counted allocation and stable address. */
typedef struct { void *resource; const void *address; } CorrectionElement;
static AmaObjectType correction_element_type;
static AmaObjectType *correction_native_types[] = {&owner_type, &correction_element_type};
static size_t correction_drops;
static bool correction_destroy_failure;

/** Construct only in fresh final storage; resource failure grants no native credit. */
static AmaStatus correction_element_default(void *payload) {
    CorrectionElement *element = payload;
    *element = (CorrectionElement){.resource = test_allocate(NULL, 1), .address = payload};
    return element->resource == NULL ? ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0) : ama_status_success();
}
/** Consume the resource even on technical Busy; cleanup must never retry it. */
static AmaStatus correction_element_destroy(void *payload) {
    CorrectionElement *element = payload;
    assert(element->resource != NULL && element->address == payload);
    test_release(NULL, element->resource);
    element->resource = NULL;
    correction_drops++;
    return correction_destroy_failure ? ama_status_make(AMA_BUSY, NULL, 0) : ama_status_success();
}
/** Specialize the still-empty owned field to this test's native nominal type. */
static void setup_correction_destruction(void) {
    setup_traversal();
    static const AmaObjectField fields[] = {
        {AMA_FIELD_REFERENCE_LIST, 1, offsetof(TraversalOwner, items)},
        {AMA_FIELD_REFERENCE, 1, offsetof(TraversalOwner, reused)},
        {AMA_FIELD_OWNING_LIST, 2, offsetof(TraversalOwner, owned)}
    };
    owner_type.fields = fields;
    owner_type.list_cursors = owned_cursors;
    correction_drops = 0;
    correction_destroy_failure = false;
    checked(ama_object_type_init(&correction_element_type, sources.allocator, 2, sizeof(CorrectionElement),
        2, UINT64_MAX, NULL, 0, 0,
        (AmaPayloadHooks){correction_element_default, NULL, NULL, correction_element_destroy}));
    world.types = correction_native_types;
    world.type_count = 2;
}
