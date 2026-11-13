/**
 * @file
 * @brief Test shared/instance selection, checked payload borrows and index failures.
 */
#include "../objects/common.c"
#include "runtime/program/values/selection/internal.h"
#include "runtime/program/calls/call.h"

static AmaProgram program;
static AmaProgramType program_types[2];
static AmaHandle root;
static size_t decodes;
static bool decoder_failure;

/** Create a real mutable instance registry, with no frontend or generated code. */
static void setup_selection(void) {
    setup_objects();
    program_types[0] = (AmaProgramType){.object = &leaf_type, .field_count = 2};
    program_types[1] = (AmaProgramType){.object = &root_type, .field_count = 6};
    program = (AmaProgram){.world = &world, .types = program_types, .type_count = 2};
    AmaStatus status = ama_object_create(&world, 2, NULL, &root);
    assert(status.code == AMA_OK);
}

/** Release only test-owned instances; shared descriptor fixtures remain borrowed. */
static void finish_selection(void) {
    AmaStatus status = ama_object_destroy(&world, root);
    assert(status.code == AMA_OK);
    destroy_objects();
}

/** Selection preserves declared form, reference locations and exact nominal identity. */
static void it_selects_declared_instance_storage_without_acquiring_reference_targets(void) {
    setup_selection();
    assert(ama_program_type(&program, 1) == &program_types[0] && ama_program_type(&program, 9) == NULL);
    RootValue *storage = object_payload(root);
    storage->selected = storage->left;
    AmaProgramValue value = {0};
    AmaProgramAccess access = {.field = 3};
    AmaStatus status = ama_program_instance_base(&program, root, &access, false, &value);
    assert(status.code == AMA_OK && value.form == AMA_FIELD_REFERENCE && value.location == &storage->selected);
    assert(ama_handle_equal(value.handle, storage->left) && ama_handle_equal(value.container, root));
    access.field = 2;
    value = (AmaProgramValue){0};
    status = ama_program_instance_base(&program, root, &access, false, &value);
    assert(status.code == AMA_OK && value.form == AMA_FIELD_OWNING_LIST && value.list == &storage->branches);
    access.field = 99;
    status = ama_program_instance_base(&program, root, &access, false, &value);
    assert(status.code == AMA_INVALID_STATE && value.list == &storage->branches);
    access.field = 0;
    AmaProgramSource source = {.kind = AMA_SOURCE_LOCAL, .access = &access, .type_id = 1, .acquire = true, .continuation = true};
    value = (AmaProgramValue){0};
    status = ama_program_evaluate(&program, root, NULL, &source, false, &value);
    assert(status.code == AMA_OK && value.form == AMA_FIELD_REFERENCE && value.continuation);
    assert(ama_handle_equal(value.handle, storage->left));
    finish_selection();
}

/** Shared scalar/list/indexed selections borrow constants and never mint handles. */
static void it_keeps_shared_values_outside_mutable_pool_identity_and_checks_shape(void) {
    setup_selection();
    NativeValue payloads[2] = {{.number = 3}, {.number = 7}};
    AmaConstantValue items[2] = {
        {.type = &program_types[0], .payload = &payloads[0]},
        {.type = &program_types[0], .payload = &payloads[1]}
    };
    AmaConstantStorage storage[] = {{.items = &items[0], .count = 1}, {.items = items, .count = 2}};
    AmaProgramConstant constants[] = {
        {.storage = &storage[0], .type_id = 1}, {.storage = &storage[1], .type_id = 1, .list = true}
    };
    program.constants = constants;
    program.constant_count = 2;
    AmaProgramAccess access = {.field = 0, .shared = true};
    AmaProgramValue value = {0};
    AmaStatus status = ama_program_select_shared(&program, NULL, &access, &value);
    assert(status.code == AMA_OK && value.constant == &items[0] && value.handle.generation == 0);
    const void *payload = NULL;
    status = ama_program_payload(&program, &value, &payload);
    assert(status.code == AMA_OK && payload == &payloads[0]);
    access.field = 1;
    status = ama_program_select_shared(&program, NULL, &access, &value);
    assert(status.code == AMA_OK && value.shared_list == &storage[1]);
    status = ama_program_index(&value, 1);
    assert(status.code == AMA_OK && value.constant == &items[1] && value.shared_list == NULL);
    AmaProgramSource source = {.kind = AMA_SOURCE_SHARED, .shared = 0, .acquire = true};
    status = ama_program_evaluate(&program, root, NULL, &source, false, &value);
    assert(status.code == AMA_INVALID_STATE);
    storage[0].count = 0;
    access.field = 0;
    status = ama_program_shared_base(&program, &access, NULL, &value);
    assert(status.code == AMA_INVALID_STATE);
    finish_selection();
}

/** A protocol failure deliberately shares a numeric bounds code with target refusal. */
static AmaStatus decode_index(const void *payload, size_t *index) {
    decodes++;
    if (decoder_failure) { return ama_status_make(AMA_INDEX_BOUNDS, NULL, 0); }
    *index = ((const NativeValue *)payload)->number;
    return ama_status_success();
}

/** Only selection bounds produce refusal; decoder errors and busy payloads remain technical. */
static void it_decodes_each_index_once_and_never_guesses_refusal_from_status_codes(void) {
    setup_selection();
    RootValue *storage = object_payload(root);
    storage->references.items = test_allocate(NULL, sizeof(AmaHandle));
    assert(storage->references.items != NULL);
    storage->references.items[0] = storage->right;
    storage->references.count = storage->references.capacity = 1;
    LeafValue *index = object_payload(storage->left);
    index->native.number = 0;
    program_types[0].index = decode_index;
    const AmaProgramAccess reader = {.field = 0};
    const AmaProgramAccess access = {.field = 5, .index = &reader};
    AmaProgramValue value = {0};
    bool refused = true;
    decodes = 0;
    decoder_failure = false;
    AmaStatus status = ama_program_select_target(&program, root, &access, &value, &refused);
    assert(status.code == AMA_OK && !refused && decodes == 1 && ama_handle_equal(value.handle, storage->right));
    index->native.number = 1;
    status = ama_program_select_target(&program, root, &access, &value, &refused);
    assert(status.code == AMA_INDEX_BOUNDS && refused && decodes == 2);
    decoder_failure = true;
    status = ama_program_select_target(&program, root, &access, &value, &refused);
    assert(status.code == AMA_INDEX_BOUNDS && !refused && decodes == 3);
    AmaPoolCell *cell = NULL;
    status = ama_pool_lookup(&leaf_type.pool, storage->left, false, &cell);
    assert(status.code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    status = ama_program_select_target(&program, root, &access, &value, &refused);
    assert(status.code == AMA_BUSY && !refused && decodes == 3);
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    status = ama_program_select(&program, root, NULL, false, &value);
    assert(status.code == AMA_INVALID_STATE);
    finish_selection();
}

static AmaStatus controlled_decoder_status;

/** Write an unusable index even on failure to expose accidental output consumption.
 * The returned transport is controlled independently of that value and message text.
 */
static AmaStatus controlled_decode_index(const void *payload, size_t *index) {
    (void)payload;
    decodes++;
    *index = SIZE_MAX;
    return controlled_decoder_status;
}

/** Only a well-formed dedicated invalid-index result records an acquisition refusal.
 * Malformed success/refusal and unrelated decoder failures preserve earlier caller history,
 * do not consume the failed output and never acquire/execute the selected target.
 */
static void it_records_only_valid_index_refusal_and_ignores_failed_conversion_outputs(void) {
    setup_selection();
    RootValue *storage = object_payload(root);
    storage->references.items = test_allocate(NULL, sizeof(AmaHandle));
    assert(storage->references.items != NULL);
    storage->references.items[0] = storage->right;
    storage->references.count = storage->references.capacity = 1;
    program_types[0].index = controlled_decode_index;
    const AmaProgramAccess reader = {.field = 0};
    const AmaProgramAccess access = {.field = 5, .index = &reader};
    const AmaProgramInvocation invocation = {.target = &access, .mode = AMA_ACCESS_ENTRY};
    AmaPoolCell *caller_cell = NULL, *target_cell = NULL;
    AmaStatus status = ama_pool_lookup(&root_type.pool, root, false, &caller_cell);
    assert(status.code == AMA_OK);
    status = ama_pool_lookup(&leaf_type.pool, storage->right, false, &target_cell);
    assert(status.code == AMA_OK);
    static const char message[] = {'b', 'u', 's', 'y'};
    const AmaStatus results[] = {
        {AMA_INVALID_INDEX, message, sizeof(message)},
        {AMA_INDEX_BOUNDS, message, sizeof(message)},
        {AMA_INVALID_INDEX, NULL, 1},
        {AMA_OK, NULL, 1},
        {AMA_OUT_OF_MEMORY, message, sizeof(message)}
    };
    const uint32_t expected[] = {AMA_INVALID_INDEX, AMA_INDEX_BOUNDS,
        AMA_INVALID_STATE, AMA_INVALID_STATE, AMA_OUT_OF_MEMORY};
    size_t constructors_before = constructed;
    decodes = 0;
    for (size_t index = 0; index < sizeof(results) / sizeof(*results); index++) {
        caller_cell->metadata.service.last_refusal = ama_status_make(AMA_BUSY, NULL, 0);
        controlled_decoder_status = results[index];
        bool refused = false;
        status = ama_program_acquire(&program, root, &invocation, &refused);
        assert(status.code == expected[index] && refused == (index == 0));
        assert(caller_cell->metadata.service.last_refusal.code == (index == 0 ? AMA_INVALID_INDEX : AMA_BUSY));
        if (index == 0) {
            assert(caller_cell->metadata.service.last_refusal.message == message);
            assert(caller_cell->metadata.service.last_refusal.message_length == sizeof(message));
        }
        assert(ama_handle_is_empty(caller_cell->metadata.service.outgoing));
        assert(target_cell->metadata.service.activity == AMA_INSTANCE_READY);
        assert(decodes == index + 1 && constructed == constructors_before);
    }
    finish_selection();
}

/**
 * Execute every named assertion case selected by this leaf; the recursive harness owns process
 * status and success-only cleanup.
 */
void run(void) {
    it_selects_declared_instance_storage_without_acquiring_reference_targets();
    it_keeps_shared_values_outside_mutable_pool_identity_and_checks_shape();
    it_decodes_each_index_once_and_never_guesses_refusal_from_status_codes();
    it_records_only_valid_index_refusal_and_ignores_failed_conversion_outputs();
}

/**
 * Run the explicit assertion suite and return success only after every selected case completes;
 * failed assertions preserve generated leaf evidence.
 */
int main(void) {
    run();
    return 0;
}
