#include "../common.c"

static void it_selects_exactly_one_constructor_and_destroys_native_before_reverse_children(void) {
    for (size_t children = 0; children <= 3; children++) {
        for (int kind = AMA_CONSTRUCT_DEFAULT; kind <= AMA_CONSTRUCT_CLONE; kind++) {
            Model model;
            Scenario scenario = {children, (AmaConstructorKind) kind, SIZE_MAX, 0};
            AmaStatus status = model_construct(&model, scenario);
            assert(ama_status_is_success(status));
            assert(model.native.constructed_by == scenario.constructor);
            assert(model.lifetime.phase == AMA_LIFETIME_LIVE);
            status = model_cleanup(&model, ama_status_success());
            assert(ama_status_is_success(status));
            assert(model.destruction_count == children + 1);
            for (size_t index = 0; index <= children; index++) {
                assert(model.destroyed[index] == children - index);
            }
            expect_finalized(&model);
        }
    }
}

static void expect_failed_prefix(size_t children, size_t failed, AmaConstructorKind kind) {
    Model model;
    Scenario scenario = {children, kind, failed, 7};
    AmaStatus status = model_construct(&model, scenario);
    assert(status.code == AMA_OUT_OF_MEMORY);
    assert(model.destruction_count == failed);
    for (size_t index = 0; index < failed; index++) {
        assert(model.destroyed[index] == failed - index - 1);
    }
    assert(model.diagnostic_count == failed);
    expect_finalized(&model);
}

static void it_cleans_every_constructed_prefix_without_destroying_failed_payloads(void) {
    for (size_t children = 0; children <= 3; children++) {
        for (size_t failed = 0; failed <= children; failed++) {
            for (int kind = AMA_CONSTRUCT_DEFAULT; kind <= AMA_CONSTRUCT_CLONE; kind++) {
                expect_failed_prefix(children, failed, (AmaConstructorKind) kind);
            }
        }
    }
}

static void it_continues_cleanup_after_every_combination_of_native_errors(void) {
    for (unsigned failures = 0; failures < 16; failures++) {
        Model model;
        Scenario scenario = {3, AMA_CONSTRUCT_LITERAL, SIZE_MAX, failures};
        AmaStatus status = model_construct(&model, scenario);
        assert(ama_status_is_success(status));
        status = model_cleanup(&model, ama_status_success());
        assert(status.code == (failures == 0 ? AMA_OK : AMA_NATIVE_FAILURE));
        assert(model.destruction_count == 4);
        expect_finalized(&model);
        status = model_cleanup(&model, ama_status_success());
        assert(status.code == AMA_INVALID_STATE && model.destruction_count == 4);
    }
}

static void it_rejects_invalid_transitions_and_never_cleans_children_before_native(void) {
    AmaLifetime lifetime = {0};
    AmaStatus status = ama_lifetime_begin(&lifetime, 1, AMA_CONSTRUCT_DEFAULT);
    assert(ama_status_is_success(status));
    status = ama_lifetime_finish(&lifetime, true);
    assert(status.code == AMA_INVALID_STATE);
    status = ama_lifetime_child_ready(&lifetime);
    assert(ama_status_is_success(status));
    status = ama_lifetime_child_ready(&lifetime);
    assert(status.code == AMA_INVALID_STATE);
    status = ama_lifetime_finish(&lifetime, true);
    assert(ama_status_is_success(status));
    status = ama_lifetime_begin_cleanup(&lifetime);
    assert(ama_status_is_success(status));
    size_t index = 99;
    bool selected = ama_lifetime_next_child(&lifetime, &index);
    assert(!selected && index == 99);
    selected = ama_lifetime_take_native(&lifetime);
    assert(selected);
    selected = ama_lifetime_take_native(&lifetime);
    assert(!selected);
    selected = ama_lifetime_next_child(&lifetime, &index);
    assert(selected && index == 0);
    status = ama_lifetime_end_cleanup(&lifetime);
    assert(ama_status_is_success(status));
}

static void it_finalizes_ama_only_storage_without_a_native_destroy_request(void) {
    AmaLifetime lifetime = {0};
    AmaStatus status = ama_lifetime_begin(&lifetime, 0, AMA_CONSTRUCT_DEFAULT);
    assert(ama_status_is_success(status));
    status = ama_lifetime_finish(&lifetime, false);
    assert(ama_status_is_success(status));
    status = ama_lifetime_begin_cleanup(&lifetime);
    assert(ama_status_is_success(status));
    bool native = ama_lifetime_take_native(&lifetime);
    assert(!native);
    status = ama_lifetime_end_cleanup(&lifetime);
    assert(ama_status_is_success(status));
}

void run(void) {
    it_selects_exactly_one_constructor_and_destroys_native_before_reverse_children();
    it_cleans_every_constructed_prefix_without_destroying_failed_payloads();
    it_continues_cleanup_after_every_combination_of_native_errors();
    it_rejects_invalid_transitions_and_never_cleans_children_before_native();
    it_finalizes_ama_only_storage_without_a_native_destroy_request();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
