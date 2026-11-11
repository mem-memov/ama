#ifndef AMA_TEST_LIFECYCLE_CONSTRUCTION_C
#define AMA_TEST_LIFECYCLE_CONSTRUCTION_C

static AmaStatus construct_native(Payload *destination, AmaConstructorKind kind) {
    if (kind == AMA_CONSTRUCT_DEFAULT) {
        return payload_initialize(destination);
    }
    const unsigned char literal[] = {'7'};
    if (kind == AMA_CONSTRUCT_LITERAL) {
        return payload_literal(destination, (AmaByteView) {literal, 1});
    }
    Payload source;
    AmaStatus status = payload_initialize(&source);
    if (!ama_status_is_success(status)) {
        return status;
    }
    status = payload_clone(destination, &source);
    AmaStatus cleanup = payload_destroy(&source);
    return ama_cleanup_primary(status, cleanup);
}

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

static AmaStatus construct_root(Model *model, Scenario scenario) {
    AmaStatus status = construct_native(&model->native, scenario.constructor);
    if (ama_status_is_success(status) && scenario.failure_ordinal == scenario.children) {
        free(model->native.resource);
        model->native.resource = NULL;
        return ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0);
    }
    return status;
}

static AmaStatus model_construct(Model *model, Scenario scenario) {
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

#endif
