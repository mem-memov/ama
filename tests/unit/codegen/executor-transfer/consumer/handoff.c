/** @file
 * @brief Make host ownership transfer explicit, effect-free on refusal and conditional on native affinity.
 * The controller excludes new commands across admission preflight and publication.
 */
/** Host decisions distinguish quiescence from native resource policy. */
typedef enum { TRANSFER_OK, TRANSFER_BUSY, TRANSFER_PINNED } TransferResult;

/** Transfer only between idle owners, without an external borrow or a pinned dependency.
 * All ordinary host callers obey this controller boundary. A transient diagnostic
 * lease is deliberately allowed in the negative case to prove core admission refusal.
 */
static TransferResult transfer_owner(unsigned target) {
    int result = pthread_mutex_lock(&controller.mutex);
    require(result == 0 && target < 2);
    TransferResult decision = TRANSFER_OK;
    if (controller.active || controller.command != COMMAND_NONE) { decision = TRANSFER_BUSY; }
    else if (controller.pinned && target != controller.owner) { decision = TRANSFER_PINNED; }
    else if (controller.host.initialized && !controller.host.disposed) {
        AmaContextLease lease = {0};
        AmaStatus status = ama_context_enter(&controller.host.registry, controller.host.identity, &lease);
        if (status.code == AMA_BUSY) { decision = TRANSFER_BUSY; }
        else {
            require(status.code == AMA_OK);
            status = ama_context_leave(&lease);
            require(status.code == AMA_OK);
        }
    }
    if (decision == TRANSFER_OK) { controller.owner = target; }
    result = pthread_mutex_unlock(&controller.mutex);
    require(result == 0);
    return decision;
}

/** Read the saved selector under a live lease, then end every view before handoff. */
static uint64_t saved_position(AmaContextLease *lease) {
    AmaStatus status = ama_context_enter(&controller.host.registry, controller.host.identity, lease);
    require(status.code == AMA_OK);
    AmaContextView view = {0};
    status = ama_context_inspect(lease, &view);
    require(status.code == AMA_OK && view.control->phase == AMA_RUNTIME_IDLE);
    AmaPoolCell *cell = NULL;
    status = ama_program_cell(view.program, controller.host.root, &cell);
    require(status.code == AMA_OK && cell->metadata.service.resume.selector != 0);
    return cell->metadata.service.resume.selector;
}
