/** @file
 * @brief Predict whole-tree destruction and clean the explicit caller after all model-owned grants and roots end.
 */
/** Destruction preflights every owned child; an acquired descendant prevents every native destructor. Successful release retires exactly the old public tree generations. */
static void model_destroy(OwnershipModel *model, size_t index) {
    OwnershipSlot *slot = &model->slots[index];
    if (!slot->live) { return; }
    size_t before = allocation_count, native_before = constructed, destroyed_before = destroyed;
    AmaStatus status = ama_object_destroy(&world, slot->handle);
    if (model_allocation_failed(model, before, status)) { return; }
    if (model->held == index) { assert(status.code == AMA_BUSY && constructed == native_before && destroyed == destroyed_before); }
    else {
        assert(status.code == AMA_OK && destroyed == destroyed_before + 3);
        retire_model_identity(model, slot->handle);
        retire_model_identity(model, slot->children[0]);
        retire_model_identity(model, slot->children[1]);
        slot->live = false;
    }
}
/** End the borrowed grant before finalizing model roots, then make the explicit caller available and release all counted resources. */
static void finish_ownership_model(OwnershipModel *model, bool injected) {
    failure_at = 0;
    model_release(model);
    model_destroy(model, 0);
    model_destroy(model, 1);
    assert_ownership_model(model);
    AmaServiceState *service = &ownership_cell(model->caller)->metadata.service;
    service->activity = AMA_INSTANCE_READY;
    service->return_link = (AmaReturnLink){0};
    AmaStatus status = ama_object_destroy(&world, model->caller);
    assert(status.code == AMA_OK && world.first_root == NULL && world.last_root == NULL);
    assert(injected ? constructed == destroyed || constructed == destroyed + 1 : constructed == destroyed);
    destroy_objects();
}
