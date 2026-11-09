#include "../common.c"

static void it_keeps_small_native_values_inline_and_delegates_literal_rules_to_the_type(void) {
    setup_test();
    uint32_t original, copy;
    AmaValue source = {&number_type, &original};
    AmaValue destination = {&number_type, &copy};
    assert(ama_value_initialize_literal(source, (const unsigned char*)"4294967295", 10) == AMA_STATUS_OK);
    assert(original == UINT32_MAX && live_allocations == 0);
    assert(ama_value_initialize_copy(destination, source) == AMA_STATUS_OK);
    original = 7;
    assert(copy == UINT32_MAX);
    assert(ama_value_replace_literal(destination, (const unsigned char*)"4294967296", 10)
        == AMA_STATUS_LITERAL_INVALID);
    assert(copy == UINT32_MAX);
    assert(ama_value_replace_literal(destination, (const unsigned char*)"1\0x", 3)
        == AMA_STATUS_LITERAL_INVALID);
    assert(copy == UINT32_MAX);
    assert(ama_value_replace_literal(destination, NULL, 0) == AMA_STATUS_LITERAL_INVALID);
    assert(ama_value_reset(destination) == AMA_STATUS_OK && copy == 0);
    ama_value_destroy(source);
    ama_value_destroy(destination);
    destroy_test();
}

static void it_copies_literal_bytes_with_an_explicit_length(void) {
    setup_test();
    BufferState original, copied;
    unsigned char bytes[] = {'a', 0, 255};
    AmaValue original_value = {&buffer_type, &original};
    AmaValue copied_value = {&buffer_type, &copied};
    assert(ama_value_initialize_literal(original_value, bytes, sizeof(bytes)) == AMA_STATUS_OK);
    bytes[0] = 'z';
    assert(original.length == 3 && original.bytes[0] == 'a' && original.bytes[1] == 0);
    assert(ama_value_initialize_copy(copied_value, original_value) == AMA_STATUS_OK);
    assert(copied.bytes != original.bytes && copied.home == &copied);
    original.bytes[0] = 'x';
    assert(copied.bytes[0] == 'a' && copied.bytes[2] == 255);
    ama_value_destroy(original_value);
    ama_value_destroy(copied_value);
    destroy_test();
}

static void it_keeps_a_live_destination_when_copying_fails(void) {
    setup_test();
    BufferState original, destination;
    AmaValue source = {&buffer_type, &original};
    AmaValue target = {&buffer_type, &destination};
    assert(ama_value_initialize(source) == AMA_STATUS_OK);
    assert(ama_value_initialize_literal(target, (const unsigned char*)"old", 3) == AMA_STATUS_OK);
    unsigned char* previous = destination.bytes;
    fail_copy_at = 1;
    assert(ama_value_replace(target, source) == AMA_STATUS_MEMORY);
    assert(destination.bytes == previous && destination.length == 3 && live_allocations == 2);
    assert(ama_value_replace(target, source) == AMA_STATUS_OK);
    assert(destination.bytes != original.bytes && destination.home == &destination);
    assert(destination.length == 7 && live_allocations == 2);
    ama_value_destroy(target);
    ama_value_destroy(source);
    destroy_test();
}

static void it_prepares_literal_replacement_before_destroying_its_source_bytes(void) {
    setup_test();
    BufferState state;
    AmaValue value = {&buffer_type, &state};
    assert(ama_value_initialize(value) == AMA_STATUS_OK);
    assert(ama_value_replace_literal(value, state.bytes + 2, state.length - 2) == AMA_STATUS_OK);
    assert(state.length == 5 && memcmp(state.bytes, "fault", 5) == 0 && state.home == &state);
    assert(ama_value_replace_literal(value, NULL, 1) == AMA_STATUS_INVALID);
    assert(state.length == 5);
    assert(ama_value_replace_literal(value, NULL, 0) == AMA_STATUS_OK && state.length == 0);
    ama_value_destroy(value);
    destroy_test();
}

static void it_preserves_the_old_value_when_reset_initialization_fails(void) {
    setup_test();
    BufferState state;
    AmaValue value = {&buffer_type, &state};
    assert(ama_value_initialize(value) == AMA_STATUS_OK);
    unsigned char* previous = state.bytes;
    fail_initialize_at = initialize_attempts + 1;
    assert(ama_value_reset(value) == AMA_STATUS_MEMORY);
    assert(state.bytes == previous && live_allocations == 1);
    assert(ama_value_reset(value) == AMA_STATUS_OK && state.home == &state && live_allocations == 1);
    ama_value_destroy(value);
    destroy_test();
}

static void it_moves_resources_and_repairs_addresses_without_copying_them(void) {
    setup_test();
    BufferState original, moved;
    AmaValue source = {&buffer_type, &original};
    AmaValue destination = {&buffer_type, &moved};
    assert(ama_value_initialize(source) == AMA_STATUS_OK);
    unsigned char* allocation = original.bytes;
    assert(ama_value_replace(source, source) == AMA_STATUS_OK && copy_attempts == 0);
    assert(ama_value_initialize_copy(source, source) == AMA_STATUS_INVALID);
    assert(ama_value_move(source, source) == AMA_STATUS_INVALID);
    assert(ama_value_move(destination, source) == AMA_STATUS_OK);
    assert(moved.bytes == allocation && moved.home == &moved && original.bytes == NULL);
    assert(copy_attempts == 0 && live_allocations == 1);
    ama_value_destroy(destination);
    destroy_test();
}

static void it_clones_nested_owned_states_and_destroys_them_in_reverse_order(void) {
    setup_test();
    TreeState original, copied;
    AmaValue source = {&tree_type, &original};
    AmaValue destination = {&tree_type, &copied};
    assert(ama_value_initialize(source) == AMA_STATUS_OK);
    original.pair.first.bytes[0] = 'a';
    original.pair.second.bytes[0] = 'b';
    original.last.bytes[0] = 'c';
    assert(ama_value_initialize_copy(destination, source) == AMA_STATUS_OK);
    assert(live_allocations == 6 && copied.pair.first.bytes != original.pair.first.bytes);
    original.pair.first.bytes[0] = 'x';
    assert(copied.pair.first.bytes[0] == 'a');
    ama_value_destroy(destination);
    assert(memcmp(destruction_order, "cba", 3) == 0);
    ama_value_destroy(source);
    assert(memcmp(destruction_order, "cbacbx", 6) == 0);
    destroy_test();
}

static void it_unwinds_nested_initialization_without_touching_uninitialized_fields(void) {
    setup_test();
    TreeState state;
    fail_initialize_at = 3;
    assert(ama_value_initialize((AmaValue){&tree_type, &state}) == AMA_STATUS_MEMORY);
    assert(live_allocations == 0 && destruction_count == 2);
    fail_initialize_at = 2;
    initialize_attempts = 0;
    assert(ama_value_initialize((AmaValue){&tree_type, &state}) == AMA_STATUS_MEMORY);
    assert(live_allocations == 0 && destruction_count == 3);
    destroy_test();
}

static void it_unwinds_nested_copy_and_preserves_both_live_trees(void) {
    setup_test();
    TreeState original, destination;
    AmaValue source = {&tree_type, &original};
    AmaValue target = {&tree_type, &destination};
    assert(ama_value_initialize(source) == AMA_STATUS_OK);
    assert(ama_value_initialize(target) == AMA_STATUS_OK);
    unsigned char* previous = destination.pair.first.bytes;
    fail_copy_at = 3;
    assert(ama_value_replace(target, source) == AMA_STATUS_MEMORY);
    assert(live_allocations == 6 && destruction_count == 2 && destination.pair.first.bytes == previous);
    assert(ama_value_replace(target, source) == AMA_STATUS_OK);
    assert(live_allocations == 6 && destination.last.home == &destination.last);
    assert(destination.pair.first.home == &destination.pair.first);
    ama_value_destroy(target);
    ama_value_destroy(source);
    destroy_test();
}

static void it_refuses_unsupported_resource_clones_and_literals(void) {
    setup_test();
    AmaType resource_type = buffer_type;
    resource_type.initialize_copy = NULL;
    resource_type.initialize_literal = NULL;
    BufferState original, destination;
    AmaValue source = {&resource_type, &original};
    AmaValue target = {&resource_type, &destination};
    assert(ama_value_initialize(source) == AMA_STATUS_OK);
    assert(ama_value_initialize(target) == AMA_STATUS_OK);
    unsigned char* previous = destination.bytes;
    assert(ama_value_replace(target, source) == AMA_STATUS_COPY_UNSUPPORTED);
    assert(ama_value_replace_literal(target, NULL, 0) == AMA_STATUS_LITERAL_UNSUPPORTED);
    assert(destination.bytes == previous && live_allocations == 2);
    AmaField fields[] = {{offsetof(PairState, first), &buffer_type},
        {offsetof(PairState, second), &resource_type}};
    AmaRecord record = {fields, 2};
    AmaType container_type = pair_type;
    container_type.context = &record;
    PairState container, copy;
    AmaValue container_value = {&container_type, &container};
    assert(ama_value_initialize(container_value) == AMA_STATUS_OK);
    assert(ama_value_initialize_copy((AmaValue){&container_type, &copy}, container_value)
        == AMA_STATUS_COPY_UNSUPPORTED);
    assert(live_allocations == 4);
    ama_value_destroy(container_value);
    ama_value_destroy(target);
    ama_value_destroy(source);
    destroy_test();
}

static void it_checks_abi_layout_alignment_and_descriptor_identity_before_callbacks(void) {
    setup_test();
    BufferState first, second;
    AmaType altered = buffer_type;
    altered.abi_version++;
    assert(ama_value_initialize((AmaValue){&altered, &first}) == AMA_STATUS_ABI_MISMATCH);
    altered = buffer_type;
    altered.alignment = 0;
    assert(ama_type_validate(&altered) == AMA_STATUS_INVALID);
    altered.alignment = _Alignof(max_align_t) * 2;
    assert(ama_type_validate(&altered) == AMA_STATUS_INVALID);
    altered = buffer_type;
    altered.move = NULL;
    assert(ama_type_validate(&altered) == AMA_STATUS_INVALID);
    assert(ama_type_validate(NULL) == AMA_STATUS_INVALID);
    assert(ama_value_initialize((AmaValue){&buffer_type, NULL}) == AMA_STATUS_INVALID);
    _Alignas(max_align_t) unsigned char storage[sizeof(BufferState) + _Alignof(max_align_t)];
    assert(ama_value_initialize((AmaValue){&buffer_type, storage + 1}) == AMA_STATUS_INVALID);
    altered = buffer_type;
    /* Even matching paths, hashes, and sizes do not authorize foreign layouts. */
    assert(ama_value_initialize_copy((AmaValue){&buffer_type, &first}, (AmaValue){&altered, &second})
        == AMA_STATUS_TYPE_MISMATCH);
    assert(initialize_attempts == 0 && copy_attempts == 0);
    destroy_test();
}

static void it_rejects_overlapping_states_and_invalid_record_layouts(void) {
    setup_test();
    _Alignas(max_align_t) unsigned char storage[sizeof(BufferState) * 2];
    assert(ama_value_initialize_copy((AmaValue){&buffer_type, storage},
        (AmaValue){&buffer_type, storage + _Alignof(BufferState)}) == AMA_STATUS_INVALID);
    PairState state;
    AmaField overlapping[] = {{0, &buffer_type}, {0, &buffer_type}};
    AmaRecord record = {overlapping, 2};
    AmaType invalid = pair_type;
    invalid.context = &record;
    assert(ama_value_initialize((AmaValue){&invalid, &state}) == AMA_STATUS_INVALID);
    overlapping[1].offset = SIZE_MAX;
    assert(ama_value_initialize((AmaValue){&invalid, &state}) == AMA_STATUS_INVALID);
    overlapping[1].offset = 1;
    assert(ama_value_initialize((AmaValue){&invalid, &state}) == AMA_STATUS_INVALID);
    assert(initialize_attempts == 0);
    destroy_test();
}

static void it_supports_empty_module_states_without_inventing_fields(void) {
    setup_test();
    AmaRecord record = {0};
    AmaType empty = pair_type;
    empty.context = &record;
    empty.size = 1;
    empty.alignment = 1;
    unsigned char first, second;
    AmaValue original = {&empty, &first};
    AmaValue copied = {&empty, &second};
    assert(ama_value_initialize(original) == AMA_STATUS_OK);
    assert(ama_value_initialize_copy(copied, original) == AMA_STATUS_OK);
    assert(ama_value_reset(copied) == AMA_STATUS_OK);
    ama_value_destroy(copied);
    ama_value_destroy(original);
    destroy_test();
}

static void run(void) {
    it_keeps_small_native_values_inline_and_delegates_literal_rules_to_the_type();
    it_copies_literal_bytes_with_an_explicit_length();
    it_keeps_a_live_destination_when_copying_fails();
    it_prepares_literal_replacement_before_destroying_its_source_bytes();
    it_preserves_the_old_value_when_reset_initialization_fails();
    it_moves_resources_and_repairs_addresses_without_copying_them();
    it_clones_nested_owned_states_and_destroys_them_in_reverse_order();
    it_unwinds_nested_initialization_without_touching_uninitialized_fields();
    it_unwinds_nested_copy_and_preserves_both_live_trees();
    it_refuses_unsupported_resource_clones_and_literals();
    it_checks_abi_layout_alignment_and_descriptor_identity_before_callbacks();
    it_rejects_overlapping_states_and_invalid_record_layouts();
    it_supports_empty_module_states_without_inventing_fields();
}

int main(void) {
    run();
    printf("unit/runtime/values: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
