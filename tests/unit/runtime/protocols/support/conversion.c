/**
 * @file
 * @brief Check condition output publication independently of raw callback writes.
 */

/** Only a checked successful condition may publish the callback's output to its caller. */
static void it_keeps_condition_outputs_and_refusal_history_unchanged_after_protocol_failure(void) {
    setup_protocols();
    context_types[0].condition = protocol_condition;
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    AmaContextReference root = create_root(&lease, view);
    assert(root_value(&lease, root)->native.resource != NULL);
    view.world->first_root->metadata.service.last_refusal = ama_status_make(AMA_BUSY, NULL, 0);
    const AmaProgramAccess access = {.field = 0};
    const AmaStatus statuses[] = {{AMA_OK, NULL, 1}, {AMA_INVALID_INDEX, NULL, 0},
        {AMA_NATIVE_FAILURE, NULL, 0}, {AMA_OK, NULL, 0}};
    const uint32_t expected[] = {AMA_INVALID_STATE, AMA_INVALID_INDEX, AMA_NATIVE_FAILURE, AMA_OK};
    for (size_t index = 0; index < sizeof(statuses) / sizeof(*statuses); index++) {
        protocol_status = statuses[index];
        bool result = false;
        AmaStatus status = ama_program_condition(view.program, root.local, &access, &result);
        assert(status.code == expected[index] && result == (index == 3));
        assert(!view.world->operation_active);
        assert(view.world->first_root->metadata.service.last_refusal.code == AMA_BUSY);
    }
    leave_context(&lease);
    finish_contexts();
}
