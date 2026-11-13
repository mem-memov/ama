/** @file
 * @brief Exhaust every length-four action word and fail every required allocation in a longer composed ownership/acquisition trace.
 */
/** Every one of 18^4 words is checked at each prefix, including ineligible no-ops, busy descendants, ordinary self-copy and stale generations. */
static void it_matches_every_bounded_ownership_acquisition_clone_and_replacement_trace(void) {
    assert(MODEL_ACTION_COUNT == 18);
    for (size_t schedule = 0; schedule < 104976; schedule++) {
        OwnershipModel model = setup_ownership_model();
        assert_ownership_model(&model);
        size_t choices = schedule;
        for (size_t step = 0; step < 4; step++) {
            ModelAction action = (ModelAction)(choices % MODEL_ACTION_COUNT);
            choices /= MODEL_ACTION_COUNT;
            apply_ownership_action(&model, action);
        }
        finish_ownership_model(&model, false);
    }
}
/** Mix successful transitions, unavailable whole trees, caller refusal provenance, retained grants, cleanup and generation reuse in one allocation-measured trace. */
static void ownership_fault_trace(OwnershipModel *model) {
    const ModelAction actions[] = {
        MODEL_CREATE_FIRST, MODEL_SEED_FIRST_RESUME, MODEL_ACQUIRE_FIRST_CHILD,
        MODEL_CLONE_FIRST_TO_SECOND, MODEL_SELF_FIRST, MODEL_PROBE_CALLER_BUSY,
        MODEL_RELEASE, MODEL_CLONE_FIRST_TO_SECOND, MODEL_REPLACE_FIRST_FROM_SECOND,
        MODEL_ACQUIRE_SECOND, MODEL_REPLACE_FIRST_FROM_SECOND, MODEL_DESTROY_SECOND,
        MODEL_RELEASE, MODEL_PROBE_CALLER_BUSY, MODEL_DESTROY_FIRST, MODEL_DESTROY_SECOND,
        MODEL_CREATE_FIRST, MODEL_DESTROY_FIRST, MODEL_CREATE_FIRST, MODEL_DESTROY_FIRST,
        MODEL_CREATE_FIRST, MODEL_DESTROY_FIRST
    };
    for (size_t index = 0; index < sizeof(actions) / sizeof(*actions); index++) { apply_ownership_action(model, actions[index]); }
}
/** Fail each mandatory allocation in the same successful prefix; the oracle adapts only to an explicitly observed injected OOM, then checks unchanged public state and eventual complete cleanup. */
static void it_preserves_the_model_at_every_composed_trace_allocation_failure(void) {
    OwnershipModel model = setup_ownership_model();
    size_t before = allocation_count;
    ownership_fault_trace(&model);
    size_t count = allocation_count - before;
    finish_ownership_model(&model, false);
    for (size_t failure = 1; failure <= count; failure++) {
        model = setup_ownership_model();
        failure_at = allocation_count + failure;
        ownership_fault_trace(&model);
        assert(model.fault_seen);
        finish_ownership_model(&model, true);
    }
}
