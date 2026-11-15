/** @file
 * @brief Prove exact native detachment, FIFO unlinking and immutable identities across reused cells.
 * [ASYNC-09](../../../../../spec/language/async-and-parallelism.md#async-09)
 * keeps old callback rights alive while making them unable to deliver into a new wait.
 */
#include "../common.c"
/** Remove each possible position from a retained FIFO while preserving every survivor's order and ownership. */
static void it_unlinks_each_completed_position_without_reordering_survivors(void) {
    size_t live = baseline();
    for (size_t removed = 0; removed < 4; removed++) {
        _Atomic unsigned retirements = 0;
        AmaNativeEventService *owner = service(4, &retirements);
        AmaNativeEventSubmission endpoints[4] = {{0}};
        for (size_t index = 0; index < 4; index++) {
            endpoints[index] = prepare(owner);
            ok(ama_native_event_service_arm(owner, &endpoints[index], pending, NULL));
            AmaNativeEventBuffer bytes = {0};
            const unsigned char payload[] = {0, (unsigned char)index, 255};
            ok(ama_native_event_buffer_copy(&endpoints[index], (AmaByteView){payload, sizeof(payload)}, &bytes));
            assert(ama_native_event_post(&endpoints[index], (int64_t)index, &bytes) == AMA_EVENT_QUEUED);
        }
        bool retired_cell = false;
        ok(ama_native_event_service_detach(owner, endpoints[removed].token, &retired_cell));
        assert(retired_cell && owner->armed == 0);
        ok(ama_native_event_service_detach(owner, endpoints[removed].token, &retired_cell));
        assert(!retired_cell && ama_native_event_post(&endpoints[removed], 99, NULL) == AMA_EVENT_STALE);
        for (size_t index = 0; index < 4; index++) {
            if (index == removed) { continue; }
            AmaNativeEventCompletion completion = {0};
            bool available = false;
            ok(ama_native_event_service_take(owner, &completion, &available));
            assert(available && completion.code == (int64_t)index && completion.buffer.length == 3);
            assert(completion.buffer.data[1] == index && completion.buffer.data[2] == 255);
            ama_native_event_completion_drop(&completion);
        }
        AmaNativeEventCompletion completion = {0};
        bool available = true;
        ok(ama_native_event_service_take(owner, &completion, &available));
        assert(!available);
        ama_native_event_service_dispose(&owner);
        for (size_t index = 0; index < 4; index++) { ama_native_event_submission_drop(&endpoints[index]); }
        assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
    }
}
/** Prepared and armed retirement release capacity but never revive old callbacks when that exact slot is reused. */
static void it_detaches_before_and_after_arm_while_old_callbacks_retain_native_storage(void) {
    size_t live = baseline();
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    AmaNativeEventSubmission old = prepare(owner);
    bool retired_cell = false;
    ok(ama_native_event_service_detach(owner, old.token, &retired_cell));
    assert(retired_cell && owner->armed == 0);
    AmaNativeEventSubmission armed = prepare(owner);
    assert(armed.token.slot == old.token.slot && armed.token.generation != old.token.generation);
    ok(ama_native_event_service_arm(owner, &armed, pending, NULL));
    ok(ama_native_event_service_detach(owner, armed.token, &retired_cell));
    assert(retired_cell && owner->armed == 0);
    AmaNativeEventReadiness readiness = {true, true};
    ok(ama_native_event_service_poll(owner, &readiness));
    assert(!readiness.ready && !readiness.pending);
    AmaNativeEventSubmission current = prepare(owner);
    ok(ama_native_event_service_arm(owner, &current, pending, NULL));
    assert(ama_native_event_post(&old, 1, NULL) == AMA_EVENT_STALE);
    assert(ama_native_event_post(&armed, 2, NULL) == AMA_EVENT_STALE);
    AmaNativeEventBuffer late = {0};
    const unsigned char payload[] = {1, 2, 3};
    ok(ama_native_event_buffer_copy(&armed, (AmaByteView){payload, sizeof(payload)}, &late));
    assert(ama_native_event_post(&armed, 3, &late) == AMA_EVENT_STALE && late.owner != NULL);
    ama_native_event_buffer_drop(&late);
    assert(ama_native_event_post(&current, 4, NULL) == AMA_EVENT_QUEUED);
    ama_native_event_service_dispose(&owner);
    assert(atomic_load(&retirements) == 0);
    ok(ama_native_event_service_detach(current.state, current.token, &retired_cell));
    assert(!retired_cell);
    ama_native_event_submission_drop(&old);
    ama_native_event_submission_drop(&armed);
    ama_native_event_submission_drop(&current);
    assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
}
/** A different service's token and malformed arguments cannot retire an unrelated active cell. */
static void it_preserves_other_services_and_failed_detachment_outputs(void) {
    size_t live = baseline();
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *left = service(1, &retirements), *right = service(1, &retirements);
    AmaNativeEventSubmission first = prepare(left), second = prepare(right);
    ok(ama_native_event_service_arm(left, &first, pending, NULL));
    bool retired_cell = true;
    ok(ama_native_event_service_detach(left, second.token, &retired_cell));
    assert(!retired_cell && left->armed == 1);
    retired_cell = true;
    AmaStatus status = ama_native_event_service_detach(NULL, first.token, &retired_cell);
    assert(status.code == AMA_INVALID_STATE && retired_cell);
    status = ama_native_event_service_detach(left, first.token, NULL);
    assert(status.code == AMA_INVALID_STATE && left->armed == 1);
    ama_native_event_service_dispose(&left);
    ama_native_event_service_dispose(&right);
    ama_native_event_submission_drop(&first);
    ama_native_event_submission_drop(&second);
    assert(atomic_load(&retirements) == 2 && atomic_load(&event_test_live) == live);
}
/** Run exact queue and stale-identity cases through actual native resource retirement. */
void run(void) {
    it_unlinks_each_completed_position_without_reordering_survivors();
    it_detaches_before_and_after_arm_while_old_callbacks_retain_native_storage();
    it_preserves_other_services_and_failed_detachment_outputs();
}
/** Exit only after every native callback right has acknowledged and all counted owners retire. */
int main(void) { run(); return 0; }
