/**
 * @file
 * @brief Check shared-value startup before roots, declaration order and failed-program cleanup.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../objects/common.c"
#include "runtime/program/program.h"

static AmaProgram program;
static AmaProgramType program_types[2];
static AmaConstantStorage shared_values[2];
static AmaProgramConstant constants[2];
static AmaProgramField leaf_declarations[2], root_declarations[6];
static const unsigned char literal_bytes[] = {37};
static const AmaProgramSource literal_source = {
    .kind = AMA_SOURCE_LITERAL, .literal = {literal_bytes, 1}, .type_id = 1
};
static const AmaProgramAccess first_field = {.field = 0};
static const AmaProgramSource first_source = {.kind = AMA_SOURCE_LOCAL, .access = &first_field, .type_id = 1};
static const AmaProgramSource shared_source = {.kind = AMA_SOURCE_SHARED, .shared = 0, .type_id = 1};
static const AmaProgramSource first_reference = {
    .kind = AMA_SOURCE_LOCAL, .access = &first_field, .type_id = 1, .acquire = true
};

/**
 * Bind explicit shared/program type descriptors to the object world fixture before startup and
 * shutdown assertions.
 */
static void setup_program(void) {
    setup_objects();
    memset(leaf_declarations, 0, sizeof(leaf_declarations));
    memset(root_declarations, 0, sizeof(root_declarations));
    memset(shared_values, 0, sizeof(shared_values));
    program_types[0] = (AmaProgramType){.object = &leaf_type, .fields = leaf_declarations, .field_count = 2};
    program_types[1] = (AmaProgramType){.object = &root_type, .fields = root_declarations, .field_count = 6};
    constants[0] = (AmaProgramConstant){.storage = &shared_values[0], .type_id = 1,
        .initializers = &literal_source, .initializer_count = 1};
    constants[1] = (AmaProgramConstant){.storage = &shared_values[1], .type_id = 1,
        .initializers = &shared_source, .initializer_count = 1};
    program = (AmaProgram){.world = &world, .types = program_types, .type_count = 2,
        .constants = constants, .constant_count = 2};
    root_declarations[0] = (AmaProgramField){.initializers = &literal_source, .initializer_count = 1};
    root_declarations[1] = (AmaProgramField){.initializers = &first_source, .initializer_count = 1};
    root_declarations[3] = (AmaProgramField){.initializers = &first_reference, .initializer_count = 1};
}

/**
 * Assert that the component initializes shared values outside pools before root creation. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_initializes_shared_values_outside_pools_before_root_creation(void) {
    setup_program();
    AmaStatus status = ama_program_constants_initialize(&program);
    assert(status.code == AMA_OK);
    assert(root_type.pool.live_count == 0 && leaf_type.pool.live_count == 0);
    assert(constructed == 2 && cloned == 1);
    NativeValue *first = shared_values[0].items[0].payload;
    NativeValue *second = shared_values[1].items[0].payload;
    assert(first != second && first->number == 37 && second->number == 37);
    status = ama_program_constants_destroy(&program, status);
    assert(status.code == AMA_OK && destroyed == constructed);
    destroy_objects();
}

/**
 * Assert that the component constructs once in field order and clones completed private fields.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_constructs_once_in_field_order_and_clones_completed_private_fields(void) {
    setup_program();
    AmaHandle root = {0};
    AmaStatus status = ama_program_start(&program, 2, &root);
    assert(status.code == AMA_OK);
    RootValue *value = object_payload(root);
    LeafValue *left = object_payload(value->left);
    LeafValue *right = object_payload(value->right);
    assert(left != right && left->native.number == 37 && right->native.number == 37);
    assert(value->selected.slot_id == value->left.slot_id && value->selected.generation == value->left.generation);
    assert(leaf_type.pool.live_count == 2 && root_type.pool.live_count == 1);
    assert(constructed == 5 && cloned == 2);
    status = ama_program_stop(&program, root, status);
    assert(status.code == AMA_OK && destroyed == constructed);
    destroy_test();
}

/**
 * Assert that the component clones shared values into owned fields without pool defaults. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_clones_shared_values_into_owned_fields_without_pool_defaults(void) {
    setup_program();
    root_declarations[0].initializers = &shared_source;
    AmaHandle root = {0};
    AmaStatus status = ama_program_start(&program, 2, &root);
    assert(status.code == AMA_OK);
    RootValue *value = object_payload(root);
    LeafValue *left = object_payload(value->left);
    assert(left->native.number == 37 && constructed == 5 && cloned == 3);
    assert(left->native.address == &left->native);
    status = ama_program_stop(&program, root, status);
    assert(status.code == AMA_OK && destroyed == constructed);
    destroy_test();
}

/**
 * Assert that the component never creates the root after constant failure. The explicit fixture
 * setup and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_never_creates_the_root_after_constant_failure(void) {
    setup_program();
    failed_constructor = 2;
    AmaHandle root = {0};
    AmaStatus status = ama_program_start(&program, 2, &root);
    assert(status.code == AMA_INVALID_LITERAL && root.generation == 0);
    assert(root_type.pool.live_count == 0 && leaf_type.pool.live_count == 0);
    status = ama_program_stop(&program, root, status);
    assert(status.code == AMA_INVALID_LITERAL && destroyed == 1);
    destroy_test();
}

/**
 * Assert that the component constructs compound constants using earlier fields and reverse
 * cleanup. The explicit fixture setup and observations define this regression; assertion failure
 * keeps its generated evidence.
 */
static void it_constructs_compound_constants_using_earlier_fields_and_reverse_cleanup(void) {
    setup_program();
    root_declarations[3] = (AmaProgramField){0};
    constants[0].type_id = 2;
    constants[0].initializers = NULL;
    constants[0].initializer_count = 0;
    constants[1].type_id = 2;
    AmaStatus status = ama_program_constants_initialize(&program);
    assert(status.code == AMA_OK && constructed == 6 && cloned == 4);
    assert(root_type.pool.live_count == 0 && leaf_type.pool.live_count == 0);
    for (size_t index = 0; index < 2; index++) {
        AmaConstantValue *value = &shared_values[index].items[0];
        NativeValue *left = value->fields[0].items[0].payload;
        NativeValue *right = value->fields[1].items[0].payload;
        assert(left != right && left->number == 37 && right->number == 37);
        assert(value->fields[3].count == 1 && value->fields[3].items == NULL);
    }
    status = ama_program_constants_destroy(&program, status);
    assert(status.code == AMA_OK && destroyed == constructed);
    destroy_objects();
}

/**
 * Assert that the component unwinds every allocation failure without leaking or publishing
 * partial roots. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_unwinds_every_allocation_failure_without_leaking_or_publishing_partial_roots(void) {
    setup_program();
    size_t start = allocation_count;
    AmaHandle root = {0};
    AmaStatus status = ama_program_start(&program, 2, &root);
    assert(status.code == AMA_OK);
    size_t count = allocation_count - start;
    status = ama_program_stop(&program, root, status);
    assert(status.code == AMA_OK);
    destroy_test();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_program();
        failure_at = allocation_count + failure;
        root = (AmaHandle){0};
        status = ama_program_start(&program, 2, &root);
        assert(status.code == AMA_OUT_OF_MEMORY && root.generation == 0);
        status = ama_program_stop(&program, root, status);
        assert(status.code == AMA_OUT_OF_MEMORY);
        destroy_test();
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_initializes_shared_values_outside_pools_before_root_creation();
    it_constructs_once_in_field_order_and_clones_completed_private_fields();
    it_clones_shared_values_into_owned_fields_without_pool_defaults();
    it_never_creates_the_root_after_constant_failure();
    it_constructs_compound_constants_using_earlier_fields_and_reverse_cleanup();
    it_unwinds_every_allocation_failure_without_leaking_or_publishing_partial_roots();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) { run(); return EXIT_SUCCESS; }
