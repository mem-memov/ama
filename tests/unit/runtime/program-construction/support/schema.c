/** @file
 * @brief Bind concrete scalar ownership/reference descriptors to real current-instance initializer sources.
 */
static AmaProgram program;
static AmaProgramType program_types[2];
static AmaProgramField leaf_declarations[2], root_declarations[6];
static const AmaProgramSource leaf_self = {.kind = AMA_SOURCE_SELF, .type_id = 1};
static const AmaProgramSource root_self = {.kind = AMA_SOURCE_SELF, .type_id = 2};
static const unsigned char literal_byte = 42;
static const AmaProgramSource literal = {.kind = AMA_SOURCE_LITERAL, .type_id = 1, .literal = {&literal_byte, 1}};
static const AmaProgramAccess left_field = {.field = 0};
static const AmaProgramSource left_reference = {.kind = AMA_SOURCE_LOCAL, .access = &left_field, .type_id = 1, .acquire = true};
/** Prepare literal/default owned children and declared self/earlier-child references, without any shared constants or implicit instances. */
static void setup_construction_program(void) {
    setup_objects();
    memset(leaf_declarations, 0, sizeof(leaf_declarations));
    memset(root_declarations, 0, sizeof(root_declarations));
    leaf_declarations[0] = (AmaProgramField){.initializers = &leaf_self, .initializer_count = 1};
    root_declarations[0] = (AmaProgramField){.initializers = &literal, .initializer_count = 1};
    root_declarations[3] = (AmaProgramField){.initializers = &left_reference, .initializer_count = 1};
    root_declarations[4] = (AmaProgramField){.initializers = &root_self, .initializer_count = 1};
    program_types[0] = (AmaProgramType){.object = &leaf_type, .fields = leaf_declarations, .field_count = 2};
    program_types[1] = (AmaProgramType){.object = &root_type, .fields = root_declarations, .field_count = 6};
    program = (AmaProgram){.world = &world, .types = program_types, .type_count = 2};
}
/** Compare final references and address-sensitive payloads after publication, without borrowing stale storage after teardown. */
static void assert_constructed_self_tree(AmaHandle handle) {
    RootValue *root = object_payload(handle);
    LeafValue *left = object_payload(root->left), *right = object_payload(root->right);
    assert(ama_handle_equal(root->self, handle) && ama_handle_equal(root->selected, root->left));
    assert(ama_handle_equal(left->peer, root->left) && ama_handle_equal(right->peer, root->right));
    assert(left->native.number == 42 && root->native.address == &root->native);
    assert(left->native.address == &left->native && right->native.address == &right->native);
    assert(root_type.pool.live_count == 1 && leaf_type.pool.live_count == 2);
}
