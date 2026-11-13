/** @file
 * @brief Use real read acquisition/release and explicit caller-refusal provenance while modeling one retained target grant.
 */
/** Acquire either the available root or its left child through declared caller reference storage; no entry preparation or body dispatch occurs. */
static void model_acquire(OwnershipModel *model, size_t index, bool child) {
    if (!model->slots[index].live || model->held != 2) { return; }
    RootValue *caller = object_payload(model->caller);
    if (child) { caller->selected = model->slots[index].children[0]; model->caller_selected = caller->selected; }
    else { caller->self = model->slots[index].handle; model->caller_self = caller->self; }
    AmaProgramAccess access = {.field = child ? 3 : 4};
    AmaProgramInvocation invocation = {.target = &access, .mode = AMA_ACCESS_READ};
    bool refused = true;
    AmaStatus status = ama_program_acquire(&program, model->caller, &invocation, &refused);
    assert(status.code == AMA_OK && !refused);
    model->held = index;
    model->child = child;
}
/** Release exactly the caller's held read target and recover the root's previous Ready/Yielded availability. */
static void model_release(OwnershipModel *model) {
    if (model->held == 2) { return; }
    AmaStatus status = ama_program_release(&program, model->caller);
    assert(status.code == AMA_OK);
    model->held = 2;
    model->child = false;
}
/** Self-targeting a Running caller is an ordinary Busy refusal; attempting a new root acquisition while it already owns a grant is a technical invalid state, not that refusal. */
static void model_probe_caller(OwnershipModel *model) {
    ((RootValue *)object_payload(model->caller))->self = model->caller;
    model->caller_self = model->caller;
    const AmaProgramAccess access = {.field = 4};
    const AmaProgramInvocation invocation = {.target = &access, .mode = AMA_ACCESS_READ};
    bool refused = true;
    AmaStatus status = ama_program_acquire(&program, model->caller, &invocation, &refused);
    if (model->held == 2) { assert(status.code == AMA_BUSY && refused); model->refusal = true; }
    else { assert(status.code == AMA_INVALID_STATE && !refused); }
}
/** Seed one opaque normalized saved marker in an available abstract service state. This does not resume a selector or execute a module body. */
static void model_seed_resume(OwnershipModel *model, size_t index) {
    OwnershipSlot *slot = &model->slots[index];
    if (!slot->live || model->held == index) { return; }
    slot->cell->metadata.service.activity = AMA_INSTANCE_YIELDED;
    slot->cell->metadata.service.resume = (AmaResumePosition){true, 7};
    slot->resume = true;
}
