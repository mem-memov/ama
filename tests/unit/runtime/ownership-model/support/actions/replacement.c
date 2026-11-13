/** @file
 * @brief Predict exact self-copy, whole-tree busy refusal and staged replacement identities/values without following implementation work records.
 */
/** A statically clonable exact self-copy is effect-free if its root is available, including a busy owned descendant; distinct replacements require both complete trees available. */
static void model_replace(OwnershipModel *model, size_t destination, size_t source) {
    OwnershipSlot *target = &model->slots[destination];
    if (!target->live || !model->slots[source].live) { return; }
    bool same = source == destination;
    bool busy = same ? model->held == destination && !model->child
        : model->held == source || model->held == destination;
    size_t before = allocation_count, native_before = constructed, destroyed_before = destroyed;
    AmaHandle old_children[2] = {target->children[0], target->children[1]};
    AmaStatus status = ama_object_replace(&world, target->handle, model->slots[source].handle, false);
    if (model_allocation_failed(model, before, status)) { return; }
    if (busy) {
        assert(status.code == AMA_BUSY && constructed == native_before && destroyed == destroyed_before);
    } else if (same) {
        assert(status.code == AMA_OK && constructed == native_before && destroyed == destroyed_before);
    } else {
        assert(status.code == AMA_OK && constructed == native_before + 3 && destroyed == destroyed_before + 3);
        size_t numbers[3];
        memcpy(numbers, model->slots[source].numbers, sizeof(numbers));
        AmaPoolCell *cell = target->cell;
        cache_ownership_slot(target, target->handle, numbers);
        assert(target->cell == cell);
        retire_model_identity(model, old_children[0]);
        retire_model_identity(model, old_children[1]);
    }
}
