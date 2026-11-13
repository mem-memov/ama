/**
 * @file
 * @brief Own counted context schemas, linear lease helpers and address-sensitive object fixtures.
 *
 * Included once by the context leaf. Descriptor arrays remain immutable while contexts live;
 * teardown closes contexts before releasing the borrowed counted allocator fixture.
 */
#ifndef AMA_CONTEXT_TEST_COMMON_C
#define AMA_CONTEXT_TEST_COMMON_C
#include <threads.h>
#include "../objects/common.c"
#include "runtime/context/internal.h"

static AmaContextRegistry registry;
static AmaContextType context_types[2];
static AmaProgramField context_leaf_fields[2], context_root_fields[6];
static const unsigned char context_literal_bytes[] = {37};
static const AmaProgramSource context_literal = {
    .kind = AMA_SOURCE_LITERAL, .literal = {context_literal_bytes, 1}, .type_id = 1
};
static const AmaContextConstant context_constant = {
    .type_id = 1, .initializers = &context_literal, .initializer_count = 1, .operation = 17
};
static AmaContextDefinition definition;

/** Initialize immutable fixture schemas using the real checked object layouts; only their
 * field/source arrays are shared. The old fixture world remains empty throughout each case.
 */
static void setup_contexts(void) {
    setup_objects();
    memset(context_leaf_fields, 0, sizeof(context_leaf_fields));
    memset(context_root_fields, 0, sizeof(context_root_fields));
    for (size_t index = 0; index < 2; index++) { context_leaf_fields[index].role = AMA_PROGRAM_STATE; }
    for (size_t index = 0; index < 6; index++) { context_root_fields[index].role = AMA_PROGRAM_STATE; }
    context_types[0] = (AmaContextType){.type_id = 1, .size = sizeof(LeafValue),
        .alignment = _Alignof(LeafValue), .slots_per_block = 2, .generation_limit = 3,
        .fields = leaf_type.fields, .declarations = context_leaf_fields, .field_count = 2,
        .native_offset = offsetof(LeafValue, native), .hooks = leaf_type.hooks, .operation = 11};
    context_types[1] = (AmaContextType){.type_id = 2, .size = sizeof(RootValue),
        .alignment = _Alignof(RootValue), .slots_per_block = 2, .generation_limit = 3,
        .fields = root_type.fields, .declarations = context_root_fields, .field_count = 6,
        .native_offset = offsetof(RootValue, native), .hooks = root_type.hooks, .operation = 12};
    definition = (AmaContextDefinition){.types = context_types, .type_count = 2,
        .constants = &context_constant, .constant_count = 1, .report = report_status};
    AmaStatus status = ama_context_registry_init(&registry, sources.allocator);
    assert(status.code == AMA_OK);
}

/** Close all context-owned resources before the counted fixture allocator expires. */
static void finish_contexts(void) {
    AmaStatus status = ama_context_registry_destroy(&registry);
    assert(status.code == AMA_OK);
    destroy_objects();
}

/** Construct exactly one context and require publication of a nonzero checked identity. */
static uint64_t create_context(void) {
    uint64_t identity = 0;
    AmaStatus status = ama_context_create(&registry, &definition, &identity);
    assert(status.code == AMA_OK && identity != 0);
    return identity;
}

/** Enter into fresh linear lease storage, then borrow this operation's independent owners. */
static AmaContextView enter_context(uint64_t identity, AmaContextLease *lease) {
    AmaStatus status = ama_context_enter(&registry, identity, lease);
    assert(status.code == AMA_OK);
    AmaContextView view = {0};
    status = ama_context_inspect(lease, &view);
    assert(status.code == AMA_OK);
    return view;
}

/** Consume one linear lease and prove its public storage is empty afterward. */
static void leave_context(AmaContextLease *lease) {
    AmaStatus status = ama_context_leave(lease);
    assert(status.code == AMA_OK && lease->context == NULL && lease->identity == 0);
}

/** Create an explicit owned root without executing a body, then wrap its local identity. */
static AmaContextReference create_root(const AmaContextLease *lease, AmaContextView view) {
    AmaHandle local = {0};
    AmaStatus status = ama_object_create(view.world, 2, NULL, &local);
    assert(status.code == AMA_OK);
    AmaContextReference reference = {0};
    status = ama_context_reference(lease, local, &reference);
    assert(status.code == AMA_OK);
    return reference;
}

/** Resolve a valid reference only under its origin lease and borrow the active native payload. */
static RootValue *root_value(const AmaContextLease *lease, AmaContextReference reference) {
    AmaObjectType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_context_resolve(lease, reference, AMA_ACCESS_READ, &type, &cell);
    assert(status.code == AMA_OK);
    return ama_pool_payload(&type->pool, cell);
}


#endif
