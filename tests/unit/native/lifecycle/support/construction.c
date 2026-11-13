/**
 * @file
 * @brief Construct only the selected native/child prefix in the lifecycle reference fixture.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_LIFECYCLE_CONSTRUCTION_C
#define AMA_TEST_LIFECYCLE_CONSTRUCTION_C

/**
 * Invoke exactly the scenario-selected default/literal/clone/size constructor; a clone source has its
 * own explicit cleanup and cannot replace the primary failure.
 */
static AmaStatus construct_native(Payload *destination, AmaConstructorKind kind) {
    if (kind == AMA_CONSTRUCT_DEFAULT) {
        return payload_initialize(destination);
    }
    const unsigned char literal[] = {'7'};
    if (kind == AMA_CONSTRUCT_LITERAL) {
        return payload_literal(destination, (AmaByteView) {literal, 1});
    }
    if (kind == AMA_CONSTRUCT_SIZE) { return payload_from_size(destination, 7); }
    Payload source;
    AmaStatus status = payload_initialize(&source);
    if (!ama_status_is_success(status)) {
        return status;
    }
    status = payload_clone(destination, &source);
    AmaStatus cleanup = payload_destroy(&source);
    return ama_cleanup_primary(status, cleanup);
}

/**
 * Construct children in order and credit only successful ones; the selected failure ordinal
 * stops before later child effects.
 */
static AmaStatus construct_children(Model *model, Scenario scenario) {
    for (size_t index = 0; index < scenario.children; index++) {
        AmaStatus status = index == scenario.failure_ordinal
            ? payload_initialize_failing(&model->children[index])
            : payload_initialize(&model->children[index]);
        if (!ama_status_is_success(status)) {
            return status;
        }
        model->children[index].fail_destroy = (scenario.cleanup_failures & (1u << index)) != 0;
        status = ama_lifetime_child_ready(&model->lifetime);
        assert(ama_status_is_success(status));
    }
    return ama_status_success();
}

/**
 * Construct the selected root resource or deliberately release its partial resource before
 * returning the injected construction failure.
 */
static AmaStatus construct_root(Model *model, Scenario scenario) {
    AmaStatus status = construct_native(&model->native, scenario.constructor);
    if (ama_status_is_success(status) && scenario.failure_ordinal == scenario.children) {
        payload_release_failed(&model->native);
        return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0);
    }
    return status;
}

/**
 * Initialize a bounded lifecycle model, construct the selected child/root prefix and unwind it
 * through the same ledger on any failure.
 */
static AmaStatus model_construct_with_failure(Model *model, Scenario scenario, size_t failed_allocation) {
    setup_test();
    failure_at = failed_allocation;
    payload_configure_allocator(sources.allocator);
    assert(scenario.children <= 3);
    memset(model, 0, sizeof(*model));
    AmaStatus status = ama_lifetime_begin(&model->lifetime, scenario.children, scenario.constructor);
    assert(ama_status_is_success(status));
    status = construct_children(model, scenario);
    if (ama_status_is_success(status)) {
        status = construct_root(model, scenario);
    }
    if (!ama_status_is_success(status)) {
        return model_cleanup(model, status);
    }
    model->native.fail_destroy = (scenario.cleanup_failures & (1u << scenario.children)) != 0;
    return ama_lifetime_finish(&model->lifetime, true);
}

/** Construct normally with no allocator failure; each case owns a fresh counted fixture. */
static AmaStatus model_construct(Model *model, Scenario scenario) {
    return model_construct_with_failure(model, scenario, 0);
}

#endif
