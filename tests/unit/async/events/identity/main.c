/** @file
 * @brief Prove nonwrapping native identity and unchanged tokens across locator/service reuse.
 * [ASYNC-05](../../../../../spec/language/async-and-parallelism.md#async-05)
 * requires immutable service/submission identity independently of native record addresses.
 */
#include "../common.c"
/** Every bounded successor is strictly newer; exhausted outputs remain untouched. */
static void it_exhausts_small_identity_spaces_without_wrap(void) {
    for (uint64_t limit = 0; limit < 8; limit++) {
        for (uint64_t current = 0; current <= limit + 1; current++) {
            uint64_t next = UINT64_MAX;
            bool reserved = ama_native_event_sequence(current, limit, &next);
            assert(reserved == (current < limit));
            assert(next == (reserved ? current + 1 : UINT64_MAX));
        }
    }
    uint64_t next = 17;
    bool reserved = ama_native_event_sequence(UINT64_MAX - 1, UINT64_MAX, &next);
    assert(reserved && next == UINT64_MAX);
    reserved = ama_native_event_sequence(UINT64_MAX, UINT64_MAX, &next);
    assert(!reserved && next == UINT64_MAX);
}
/** Fail service, cell and first-domain allocations before any image receipt has been published. */
static void it_unwinds_every_first_service_allocation_before_publishing_identity(void) {
    assert(atomic_load(&event_test_live) == 0);
    for (size_t failure = 1; failure <= 3; failure++) {
        _Atomic unsigned retirements = 0;
        AmaNativeEventKeepalive keepalive = {&retirements, retired};
        AmaNativeEventService *owner = NULL;
        atomic_store(&event_test_fail, atomic_load(&event_test_allocations) + failure);
        AmaStatus status = ama_native_event_service_create(1, &keepalive, &owner);
        atomic_store(&event_test_fail, 0);
        assert(status.code == AMA_OUT_OF_MEMORY && owner == NULL);
        assert(keepalive.resource == &retirements && atomic_load(&retirements) == 0);
        assert(atomic_load(&event_test_live) == 0);
        keepalive.release(keepalive.resource);
    }
}
/** Locator zero is reused while the old callback lease remains valid; its token never authenticates the new record. */
static void it_rejects_old_tokens_after_reusing_the_exact_native_cell(void) {
    size_t live = baseline();
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    AmaNativeEventSubmission first = prepare(owner);
    ok(ama_native_event_service_arm(owner, &first, pending, NULL));
    AmaNativeEventPostResult posted = ama_native_event_post(&first, 10, NULL);
    assert(posted == AMA_EVENT_QUEUED);
    AmaNativeEventCompletion completion = {0};
    bool available = false;
    ok(ama_native_event_service_take(owner, &completion, &available));
    assert(available && ama_native_event_token_equal(first.token, completion.token));
    ama_native_event_completion_drop(&completion);
    AmaNativeEventSubmission second = prepare(owner);
    assert(second.token.slot == first.token.slot && second.token.generation > first.token.generation);
    ok(ama_native_event_service_arm(owner, &second, pending, NULL));
    posted = ama_native_event_post(&first, 11, NULL);
    assert(posted == AMA_EVENT_STALE);
    posted = ama_native_event_post(&second, 20, NULL);
    assert(posted == AMA_EVENT_QUEUED);
    ok(ama_native_event_service_take(owner, &completion, &available));
    assert(available && completion.code == 20 && ama_native_event_token_equal(second.token, completion.token));
    ama_native_event_completion_drop(&completion);
    ama_native_event_service_dispose(&owner);
    assert(atomic_load(&retirements) == 0);
    ama_native_event_submission_drop(&first);
    ama_native_event_submission_drop(&second);
    assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
}
/** A new service restarts its submission count but has a distinct authenticated service identity. */
static void it_never_revives_a_disposed_service_token(void) {
    size_t live = baseline();
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    AmaNativeEventSubmission old = prepare(owner);
    AmaNativeEventToken token = old.token;
    ama_native_event_service_dispose(&owner);
    ama_native_event_submission_drop(&old);
    owner = service(1, &retirements);
    AmaNativeEventSubmission current = prepare(owner);
    assert(current.token.generation == token.generation && current.token.slot == token.slot);
    assert(current.token.domain != token.domain || current.token.service != token.service);
    ok(ama_native_event_service_arm(owner, &current, pending, NULL));
    AmaNativeEventSubmission borrowed = current;
    borrowed.token = token;
    AmaNativeEventPostResult posted = ama_native_event_post(&borrowed, 1, NULL);
    assert(posted == AMA_EVENT_STALE);
    ama_native_event_service_dispose(&owner);
    ama_native_event_submission_drop(&current);
    assert(atomic_load(&retirements) == 2 && atomic_load(&event_test_live) == live);
}
/** Seed exact private boundaries, then require failed reservation/clone to preserve all published metadata. */
static void it_preserves_state_when_submission_or_reference_identity_is_exhausted(void) {
    size_t live = baseline();
    _Atomic unsigned retirements = 0;
    AmaNativeEventService *owner = service(1, &retirements);
    owner->generation = UINT64_MAX;
    size_t free = owner->free;
    AmaNativeEventSubmission output = {0};
    AmaStatus status = ama_native_event_service_prepare(owner, &output);
    assert(status.code == AMA_LIMIT_EXCEEDED && output.state == NULL && owner->free == free && owner->generation == UINT64_MAX);
    owner->generation = 0;
    AmaNativeEventSubmission current = prepare(owner);
    size_t references = owner->references;
    owner->references = SIZE_MAX;
    status = ama_native_event_submission_clone(&current, &output);
    assert(status.code == AMA_LIMIT_EXCEEDED && output.state == NULL && owner->references == SIZE_MAX);
    owner->references = references;
    ama_native_event_service_dispose(&owner);
    ama_native_event_submission_drop(&current);
    assert(atomic_load(&retirements) == 1 && atomic_load(&event_test_live) == live);
}
/** Run exact arithmetic, reuse and lifetime boundaries independently of an Ama runtime. */
void run(void) {
    it_exhausts_small_identity_spaces_without_wrap();
    it_unwinds_every_first_service_allocation_before_publishing_identity();
    it_rejects_old_tokens_after_reusing_the_exact_native_cell();
    it_never_revives_a_disposed_service_token();
    it_preserves_state_when_submission_or_reference_identity_is_exhausted();
}
/** Return after each immutable identity/resource lifetime has been verified. */
int main(void) { run(); return 0; }
