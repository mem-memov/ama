/** @file
 * @brief Check exact shared dependency values, independent resources and an explicit reverse cleanup oracle.
 */
/** No constant allocation implies an executable instance: all pools remain empty and no independent root or execution root exists. */
static void assert_no_instances(AmaContextView view) {
    assert(view.world->type_count == 2 && view.program->type_count == 2 && view.program->constant_count == 3);
    assert(view.world->first_root == NULL && view.world->last_root == NULL && ama_handle_is_empty(view.control->root));
    for (size_t index = 0; index < 2; index++) {
        const AmaPool *pool = &view.world->types[index]->pool;
        assert(pool->block_count == 0 && pool->allocated_blocks == 0 && pool->live_count == 0);
    }
}
/** Validate separate constant-owned arrays/native resources and the dependency-selected values without reading a pool slot. */
static void assert_shared_values(AmaContextView view) {
    const AmaConstantValue *first = view.program->constants[0].storage->items;
    const NativeValue *leaf = first->payload;
    assert(first->type->object->pool.type_id == 1 && first->native_live && leaf->number == 37 && leaf->address == leaf);
    for (size_t declaration = 1; declaration < 3; declaration++) {
        const AmaConstantStorage *storage = view.program->constants[declaration].storage;
        assert(storage->count == 1);
        const AmaConstantValue *value = storage->items;
        const NativeValue *native = value->payload;
        assert(value->type->object->pool.type_id == 2 && value->native_live && native->number == 0 && native->address == native);
        assert(value->fields[0].count == 1 && value->fields[1].count == 1);
        const AmaConstantValue *left = &value->fields[0].items[0], *right = &value->fields[1].items[0];
        const NativeValue *a = left->payload, *b = right->payload;
        assert(left->parent == value && right->parent == value && left != right);
        assert(a->number == 37 && b->number == 37 && a->resource != b->resource);
        assert(a->resource != leaf->resource && b->resource != leaf->resource);
        assert(a->address == a && b->address == b);
        assert(value->fields[3].items == NULL && value->fields[4].items == NULL);
        assert(value->fields[2].count == 0 && value->fields[5].count == 0);
    }
    AmaProgramAccess access = {.shared = true, .field = 1};
    AmaProgramValue selected = {0};
    AmaStatus status = ama_program_select_shared(view.program, NULL, &access, &selected);
    assert(status.code == AMA_OK && selected.constant == view.program->constants[1].storage->items);
    assert(selected.location == NULL && ama_handle_is_empty(selected.handle));
    AmaProgramSource acquisition = {.kind = AMA_SOURCE_SHARED, .shared = 1, .type_id = 2, .acquire = true};
    status = ama_program_evaluate(view.program, (AmaHandle){0}, NULL, &acquisition, false, &selected);
    assert(status.code == AMA_INVALID_STATE);
    const AmaConstantValue *original = view.program->constants[1].storage->items;
    const AmaConstantValue *copy = view.program->constants[2].storage->items;
    assert(original != copy && original->payload != copy->payload && original->fields != copy->fields);
    assert(((const NativeValue *)original->payload)->resource != ((const NativeValue *)copy->payload)->resource);
}
/** Capture the known native-before-reverse-children/reverse-declaration order without traversing the implementation cleanup links. */
static size_t expected_shared_cleanup(AmaContextView view, uintptr_t expected[7]) {
    size_t count = 0;
    for (size_t declaration = 3; declaration > 1;) {
        const AmaConstantValue *value = view.program->constants[--declaration].storage->items;
        expected[count++] = (uintptr_t)value->payload;
        expected[count++] = (uintptr_t)value->fields[1].items[0].payload;
        expected[count++] = (uintptr_t)value->fields[0].items[0].payload;
    }
    expected[count++] = (uintptr_t)view.program->constants[0].storage->items[0].payload;
    assert(count == 7);
    return count;
}
/** Compare distinct mutable state allocations across worlds while retaining equal logical values and shared immutable field schema. */
static void assert_distinct_shared_storage(AmaContextView left, AmaContextView right) {
    assert(left.world != right.world && left.program != right.program);
    for (size_t declaration = 0; declaration < 3; declaration++) {
        const AmaConstantValue *a = left.program->constants[declaration].storage->items;
        const AmaConstantValue *b = right.program->constants[declaration].storage->items;
        assert(a != b && a->type != b->type && a->type->object != b->type->object);
        assert(a->type->object->fields == b->type->object->fields && a->payload != b->payload && a->fields != b->fields);
        assert(((const NativeValue *)a->payload)->resource != ((const NativeValue *)b->payload)->resource);
        if (declaration != 0) {
            for (size_t field = 0; field < 2; field++) {
                const AmaConstantValue *x = a->fields[field].items, *y = b->fields[field].items;
                assert(x != y && x->payload != y->payload);
                assert(((const NativeValue *)x->payload)->resource != ((const NativeValue *)y->payload)->resource);
            }
        }
    }
}
