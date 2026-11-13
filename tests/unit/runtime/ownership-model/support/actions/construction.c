/** @file
 * @brief Predict explicit creation and independent clone outcomes, including busy descendant boundaries and required allocation failure.
 */
/** Create into an empty model position; native default numbers follow the fixture's declared child-before-parent construction order. */
static void model_create(OwnershipModel *model, size_t index) {
    if (model->slots[index].live) { return; }
    size_t native_before = constructed, before = allocation_count;
    AmaHandle output = {99, 99, 99};
    AmaStatus status = ama_object_create(&world, 2, NULL, &output);
    if (model_allocation_failed(model, before, status)) { assert(ama_handle_equal(output, (AmaHandle){99, 99, 99})); return; }
    assert(status.code == AMA_OK);
    const size_t numbers[] = {native_before + 3, 42, native_before + 2};
    cache_ownership_slot(&model->slots[index], output, numbers);
}
/** Clone only into an empty position. Any held root/child inside the source tree prevents copying before native effects. */
static void model_clone(OwnershipModel *model, size_t source, size_t destination) {
    if (!model->slots[source].live || model->slots[destination].live) { return; }
    size_t before = allocation_count, native_before = constructed, destroyed_before = destroyed;
    AmaHandle output = {99, 99, 99};
    AmaStatus status = ama_object_clone(&world, model->slots[source].handle, false, &output);
    if (model_allocation_failed(model, before, status)) { assert(ama_handle_equal(output, (AmaHandle){99, 99, 99})); return; }
    if (model->held == source) {
        assert(status.code == AMA_BUSY && constructed == native_before && destroyed == destroyed_before);
        assert(ama_handle_equal(output, (AmaHandle){99, 99, 99}));
    } else {
        assert(status.code == AMA_OK && constructed == native_before + 3);
        size_t numbers[3];
        memcpy(numbers, model->slots[source].numbers, sizeof(numbers));
        cache_ownership_slot(&model->slots[destination], output, numbers);
    }
}
