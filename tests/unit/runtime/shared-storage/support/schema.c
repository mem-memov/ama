/** @file
 * @brief Define three dependency-ordered scalar shared declarations using immutable native values and one owned-child pair.
 */
static AmaContextConstant shared_declarations[3];
static const AmaProgramSource first_shared = {.kind = AMA_SOURCE_SHARED, .shared = 0, .type_id = 1};
static const AmaProgramSource second_shared = {.kind = AMA_SOURCE_SHARED, .shared = 1, .type_id = 2};
static const AmaProgramAccess first_owned = {.field = 0};
static const AmaProgramSource earlier_owned = {.kind = AMA_SOURCE_LOCAL, .access = &first_owned, .type_id = 1};
static uintptr_t cleanup_addresses[100];
static size_t cleanup_count, shared_defaults, shared_literals;
/** Give default constants a context-independent logical value while preserving counted native resources and constructor failure injection. */
static AmaStatus shared_default(void *address) {
    shared_defaults++;
    AmaStatus status = initialize_payload(address);
    if (status.code == AMA_OK) { ((NativeValue *)address)->number = 0; }
    return status;
}
/** Observe direct literal selection without introducing default-then-literal construction. */
static AmaStatus shared_literal(void *address, AmaByteView bytes) {
    shared_literals++;
    return literal_payload(address, bytes);
}
/** Record native address before consuming its resource; integers remain comparable after the payload allocation is released. */
static AmaStatus shared_destroy(void *address) {
    assert(cleanup_count < 100);
    cleanup_addresses[cleanup_count++] = (uintptr_t)address;
    return destroy_payload(address);
}
/** Borrow one immutable schema in any number of contexts: literal leaf, compound value using that leaf and an earlier owned field, then a whole compound copy. */
static void setup_shared(void) {
    setup_contexts();
    cleanup_count = shared_defaults = shared_literals = 0;
    context_root_fields[0] = (AmaProgramField){.role = AMA_PROGRAM_STATE, .initializers = &first_shared, .initializer_count = 1};
    context_root_fields[1] = (AmaProgramField){.role = AMA_PROGRAM_STATE, .initializers = &earlier_owned, .initializer_count = 1};
    shared_declarations[0] = context_constant;
    shared_declarations[1] = (AmaContextConstant){.type_id = 2, .operation = 18};
    shared_declarations[2] = (AmaContextConstant){.type_id = 2, .initializers = &second_shared, .initializer_count = 1, .operation = 19};
    definition.constants = shared_declarations;
    definition.constant_count = 3;
    for (size_t index = 0; index < 2; index++) {
        context_types[index].hooks.initialize = shared_default;
        context_types[index].hooks.literal = shared_literal;
        context_types[index].hooks.destroy = shared_destroy;
    }
}
