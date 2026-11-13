/** @file
 * @brief Dispatch a finite exact action alphabet and check complete ownership/service invariants after every transition.
 */
/** Apply only this action's documented eligible transition; the scheduler's empty/ineligible cases are explicit no-ops. */
static void apply_ownership_action(OwnershipModel *model, ModelAction action) {
    switch (action) {
        case MODEL_CREATE_FIRST: model_create(model, 0); break;
        case MODEL_CREATE_SECOND: model_create(model, 1); break;
        case MODEL_CLONE_FIRST_TO_SECOND: model_clone(model, 0, 1); break;
        case MODEL_CLONE_SECOND_TO_FIRST: model_clone(model, 1, 0); break;
        case MODEL_REPLACE_FIRST_FROM_SECOND: model_replace(model, 0, 1); break;
        case MODEL_REPLACE_SECOND_FROM_FIRST: model_replace(model, 1, 0); break;
        case MODEL_SELF_FIRST: model_replace(model, 0, 0); break;
        case MODEL_SELF_SECOND: model_replace(model, 1, 1); break;
        case MODEL_ACQUIRE_FIRST: model_acquire(model, 0, false); break;
        case MODEL_ACQUIRE_FIRST_CHILD: model_acquire(model, 0, true); break;
        case MODEL_ACQUIRE_SECOND: model_acquire(model, 1, false); break;
        case MODEL_ACQUIRE_SECOND_CHILD: model_acquire(model, 1, true); break;
        case MODEL_RELEASE: model_release(model); break;
        case MODEL_DESTROY_FIRST: model_destroy(model, 0); break;
        case MODEL_DESTROY_SECOND: model_destroy(model, 1); break;
        case MODEL_SEED_FIRST_RESUME: model_seed_resume(model, 0); break;
        case MODEL_SEED_SECOND_RESUME: model_seed_resume(model, 1); break;
        case MODEL_PROBE_CALLER_BUSY: model_probe_caller(model); break;
        case MODEL_ACTION_COUNT: assert(false); break;
    }
    assert_ownership_model(model);
}
