/**
 * @file
 * @brief Consume lifecycle fixture resource credits and retain every secondary cleanup failure.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_LIFECYCLE_CLEANUP_C
#define AMA_TEST_LIFECYCLE_CLEANUP_C

/**
 * Record every non-success cleanup status in the bounded fixture diagnostics and preserve the
 * first primary failure.
 */
static AmaStatus observe_cleanup(Model *model, AmaStatus primary, AmaStatus additional) {
    if (!ama_status_is_success(additional)) {
        assert(model->diagnostic_count < 4);
        model->diagnostics[model->diagnostic_count++] = additional;
    }
    return ama_cleanup_primary(primary, additional);
}

/**
 * Consume native credit then reverse initialized child credits, observe every destructor result
 * and finish the ledger without allocating.
 */
static AmaStatus model_cleanup(Model *model, AmaStatus primary) {
    AmaStatus status = ama_lifetime_begin_cleanup(&model->lifetime);
    if (!ama_status_is_success(status)) {
        return status;
    }
    if (ama_lifetime_take_native(&model->lifetime)) {
        model->destroyed[model->destruction_count++] = model->lifetime.child_capacity;
        status = payload_destroy(&model->native);
        primary = observe_cleanup(model, primary, status);
    }
    size_t index;
    while (ama_lifetime_next_child(&model->lifetime, &index)) {
        model->destroyed[model->destruction_count++] = index;
        status = payload_destroy(&model->children[index]);
        primary = observe_cleanup(model, primary, status);
    }
    status = ama_lifetime_end_cleanup(&model->lifetime);
    assert(ama_status_is_success(status));
    return primary;
}

#endif
