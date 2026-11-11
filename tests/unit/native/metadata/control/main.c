#include "../common.c"

static void it_checks_type_liveness_slot_and_generation_before_access(void) {
    setup_test();
    AmaHandle handle = {7, 0, 1};
    AmaStatus status = ama_validate_handle(handle, 7, &caller.identity);
    assert(ama_status_is_success(status));
    status = ama_validate_handle(handle, 8, &caller.identity);
    assert(status.code == AMA_TYPE_MISMATCH);
    handle.generation = 2;
    status = ama_validate_handle(handle, 7, &caller.identity);
    assert(status.code == AMA_STALE_REFERENCE);
    status = ama_validate_handle((AmaHandle) {0}, 7, NULL);
    assert(status.code == AMA_EMPTY_REFERENCE);
    status = ama_validate_handle((AmaHandle) {7, 1, 1}, 7, &caller.identity);
    assert(status.code == AMA_STALE_REFERENCE);
    caller.identity.live = false;
    status = ama_validate_handle((AmaHandle) {7, 0, 1}, 7, &caller.identity);
    assert(status.code == AMA_STALE_REFERENCE);
    destroy_test();
}

static void it_retires_exhausted_generations_without_wrap_or_resurrection(void) {
    AmaSlotIdentity slot = {7, 0, UINT64_MAX - 1, false, false};
    AmaStatus status = ama_slot_advance_generation(&slot);
    assert(ama_status_is_success(status) && slot.generation == UINT64_MAX);
    status = ama_slot_advance_generation(&slot);
    assert(status.code == AMA_LIMIT_EXCEEDED && slot.retired);
    status = ama_slot_advance_generation(&slot);
    assert(status.code == AMA_STALE_REFERENCE && slot.generation == UINT64_MAX);
    slot = (AmaSlotIdentity) {7, 0, 0, false, false};
    status = ama_slot_advance_generation(&slot);
    assert(ama_status_is_success(status) && slot.generation == 1);
    slot.live = true;
    status = ama_slot_advance_generation(&slot);
    assert(status.code == AMA_BUSY && slot.generation == 1);
}

static void it_refuses_busy_instances_but_allows_another_instance_of_the_same_type(void) {
    setup_test();
    const AmaInstanceActivity states[] = {
        AMA_INSTANCE_RUNNING, AMA_INSTANCE_WAITING, AMA_INSTANCE_ACQUIRED
    };
    for (size_t index = 0; index < 3; index++) {
        caller.service.activity = states[index];
        for (int mode = AMA_ACCESS_ENTRY; mode <= AMA_ACCESS_READ; mode++) {
            AmaStatus status = ama_instance_access(&caller, (AmaAccessMode) mode);
            assert(status.code == AMA_BUSY);
            status = ama_instance_access(&callee, AMA_ACCESS_ENTRY);
            assert(ama_status_is_success(status));
        }
    }
    callee.identity.live = false;
    AmaStatus status = ama_instance_access(&callee, AMA_ACCESS_READ);
    assert(status.code == AMA_STALE_REFERENCE);
    destroy_test();
}

static void it_keeps_return_connections_and_resume_positions_separate(void) {
    setup_test();
    caller.service.activity = AMA_INSTANCE_WAITING;
    callee.service.activity = AMA_INSTANCE_RUNNING;
    callee.service.return_link = (AmaReturnLink) {true, false, {7, 0, 1}, 11, 22};
    callee.service.current_operation = 33;
    assert(callee.service.return_link.caller.slot_id == caller.identity.slot_id);
    assert(callee.service.return_link.continuation == 11 && !callee.service.resume.valid);
    assert(ama_return_link_is_valid(callee.service.return_link));
    AmaReturnLink host = {true, true, {0}, 0, 0};
    assert(ama_return_link_is_valid(host));
    host.caller = (AmaHandle) {7, 0, 1};
    assert(!ama_return_link_is_valid(host));
    assert(callee.service.current_operation != callee.service.return_link.call_site);
    destroy_test();
}

static void it_preserves_caller_refusals_and_exposes_only_a_value_snapshot(void) {
    setup_test();
    AmaStatus status = ama_service_record_refusal(&caller.service,
        ama_status_make(AMA_BUSY, NULL, 0));
    assert(ama_status_is_success(status));
    caller.service.activity = AMA_INSTANCE_WAITING;
    AmaNativeServices view = ama_native_services_snapshot(&caller.service);
    caller.service.activity = AMA_INSTANCE_READY;
    status = ama_service_record_refusal(&callee.service,
        ama_status_make(AMA_INVALID_INDEX, NULL, 0));
    assert(ama_status_is_success(status) && caller.service.last_refusal.code == AMA_BUSY);
    view.caller_refusal.code = AMA_OK;
    assert(caller.service.last_refusal.code == AMA_BUSY);
    status = ama_instance_access(&caller, AMA_ACCESS_READ);
    assert(ama_status_is_success(status) && caller.service.last_refusal.code == AMA_BUSY);
    status = ama_service_record_refusal(&caller.service,
        ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0));
    assert(status.code == AMA_INVALID_STATE && caller.service.last_refusal.code == AMA_BUSY);
    view = ama_native_services_snapshot(NULL);
    assert(ama_status_is_success(view.caller_refusal));
    destroy_test();
}

static void it_copies_only_permitted_continuation_state_into_fresh_service_contexts(void) {
    setup_test();
    caller.service.activity = AMA_INSTANCE_YIELDED;
    caller.service.resume = (AmaResumePosition) {true, 44};
    caller.service.last_refusal = ama_status_make(AMA_BUSY, NULL, 0);
    caller.service.current_operation = 55;
    for (int preserve = 0; preserve <= 1; preserve++) {
        AmaStatus status = ama_service_clone(&caller.service, preserve != 0, &callee.service);
        assert(ama_status_is_success(status));
        assert(callee.service.resume.valid == (preserve != 0));
        assert(callee.service.resume.selector == (preserve ? 44 : 0));
        assert(!callee.service.return_link.attached && callee.service.current_operation == 0);
        assert(ama_status_is_success(callee.service.last_refusal));
        assert(callee.identity.slot_id == 1 && caller.service.last_refusal.code == AMA_BUSY);
    }
    caller.service.activity = AMA_INSTANCE_WAITING;
    callee.service.current_operation = 99;
    AmaStatus status = ama_service_clone(&caller.service, true, &callee.service);
    assert(status.code == AMA_BUSY && callee.service.current_operation == 99);
    status = ama_service_clone(&caller.service, true, &caller.service);
    assert(status.code == AMA_INVALID_STATE && caller.service.current_operation == 55);
    destroy_test();
}

static void it_requires_a_saved_position_for_resume_and_never_resumes_failed_execution(void) {
    setup_test();
    AmaStatus status = ama_instance_access(&caller, AMA_ACCESS_RESUME);
    assert(status.code == AMA_NO_RESUME);
    caller.service.return_link.host = true;
    status = ama_instance_access(&caller, AMA_ACCESS_READ);
    assert(status.code == AMA_INVALID_STATE);
    caller.service.return_link = (AmaReturnLink) {0};
    caller.service.activity = AMA_INSTANCE_YIELDED;
    caller.service.resume = (AmaResumePosition) {true, 8};
    status = ama_instance_access(&caller, AMA_ACCESS_RESUME);
    assert(ama_status_is_success(status));
    caller.service.activity = AMA_INSTANCE_FAILED;
    status = ama_instance_access(&caller, AMA_ACCESS_RESUME);
    assert(status.code == AMA_INVALID_STATE);
    destroy_test();
}

static void it_distinguishes_all_host_outcomes_and_initialization_failure(void) {
    AmaHostResult result = {AMA_HOST_COMPLETED, {7, 0, 1}, {0}};
    assert(ama_host_result_is_valid(result));
    result.outcome = AMA_HOST_YIELDED;
    assert(ama_host_result_is_valid(result));
    result.outcome = AMA_HOST_FAILED;
    assert(!ama_host_result_is_valid(result));
    result.status = ama_status_make(AMA_OUT_OF_MEMORY, NULL, 0);
    result.root = (AmaHandle) {0};
    assert(ama_host_result_is_valid(result));
    result.outcome = AMA_HOST_YIELDED;
    assert(!ama_host_result_is_valid(result));
    result.outcome = AMA_HOST_FAILED;
    result.root = (AmaHandle) {0, 1, 0};
    assert(!ama_host_result_is_valid(result));
}

void run(void) {
    it_checks_type_liveness_slot_and_generation_before_access();
    it_retires_exhausted_generations_without_wrap_or_resurrection();
    it_refuses_busy_instances_but_allows_another_instance_of_the_same_type();
    it_keeps_return_connections_and_resume_positions_separate();
    it_preserves_caller_refusals_and_exposes_only_a_value_snapshot();
    it_copies_only_permitted_continuation_state_into_fresh_service_contexts();
    it_requires_a_saved_position_for_resume_and_never_resumes_failed_execution();
    it_distinguishes_all_host_outcomes_and_initialization_failure();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
