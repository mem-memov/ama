/** @file
 * @brief Execute cancellation requests, real typed queue wakeups and single terminal commits through compiled Ama programs.
 */
#include "../execution/common.c"

/** Load a maintained source with a bounded exact read; generated C is never an expected-output oracle. */
static void cancellation_source(const char *path, char source[32768]) {
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    size_t count = fread(source, 1, 32767, input);
    assert(!ferror(input) && feof(input));
    source[count] = '\0';
    int closed = fclose(input);
    assert(closed == 0);
}

/** Observe ordinary result predicates after Ama-side assertions exclude residual waits, membership and mailbox bytes. */
static bool cancellation_summary(FILE *body, const ScaffoldType *type) {
    if (strcmp(type->plan->type->module->source->logical_path, "/test/fixture/v1.0.0/src/summary") != 0) { return false; }
    int written = fputs("printf(\"outcome=%zu%zu%zu\\n\",parameters->AMA_INPUT(0)->payload->value,"
        "parameters->AMA_INPUT(1)->payload->value,parameters->AMA_INPUT(2)->payload->value);\n", body);
    assert(written != EOF);
    return true;
}

/** Compose the previously tested queue with explicitly replaced record/task/cancellation operations. */
static void cancellation_project(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    directory("generated/project/packages/test/fixture/v1.0.0/src/kind");
    const char *shared[] = {"queue", "entry", "enqueue", "pop", "step", "dispatch"};
    const char *local[] = {"record", "task", "cancel", "deliver"};
    char source[32768], input[256], output[256];
    for (size_t index = 0; index < sizeof(shared) / sizeof(*shared); index++) {
        int length = snprintf(input, sizeof(input), "../ready-queues/project/kind/%s", shared[index]);
        assert(length > 0 && (size_t)length < sizeof(input));
        cancellation_source(input, source);
        length = snprintf(output, sizeof(output), "generated/project/packages/test/fixture/v1.0.0/src/kind/%s", shared[index]);
        assert(length > 0 && (size_t)length < sizeof(output));
        file(output, source);
    }
    for (size_t index = 0; index < sizeof(local) / sizeof(*local); index++) {
        int length = snprintf(input, sizeof(input), "project/kind/%s", local[index]);
        assert(length > 0 && (size_t)length < sizeof(input));
        cancellation_source(input, source);
        length = snprintf(output, sizeof(output), "generated/project/packages/test/fixture/v1.0.0/src/kind/%s", local[index]);
        assert(length > 0 && (size_t)length < sizeof(output));
        file(output, source);
    }
    cancellation_source("../scheduler-rounds/project/kind/validate", source);
    file("generated/project/packages/test/fixture/v1.0.0/src/kind/validate", source);
    cancellation_source("project/mailbox", source);
    file("generated/project/packages/test/fixture/v1.0.0/src/mailbox", source);
    cancellation_source("project/report", source);
    file("generated/project/packages/test/fixture/v1.0.0/src/report", source);
    native_fixture("summary", "@<\npending\n//std/condition\ncancelled\n//std/condition\ncompleted\n//std/condition\n@C\n");
    native_entry_override = cancellation_summary;
}

/** Execute one explicitly ordered schedule and compare its observable terminal outcome exactly. */
static void cancellation_schedule(const char *scenario, const char *expected) {
    cancellation_project();
    char setup[32768], tail[32768], source[65536], path[256];
    cancellation_source("project/setup", setup);
    int length = snprintf(path, sizeof(path), "project/%s", scenario);
    assert(length > 0 && (size_t)length < sizeof(path));
    cancellation_source(path, tail);
    length = snprintf(source, sizeof(source), "%s%s", setup, tail);
    assert(length > 0 && (size_t)length < sizeof(source));
    fprintf(stderr, "cancellation scenario: %s\n", scenario);
    execute_project(source, expected, true);
    expect_diagnostic("");
}

/** An already requested cancellation is observed before the first Awaiting boundary. */
static void it_commits_cold_cancellation_without_losing_the_initial_wake(void) {
    cancellation_schedule("cold-cancel", "outcome=010\n");
}
/** An awaiting task is queued once and late callback bytes cannot revive it or overwrite cancellation. */
static void it_wakes_once_and_rejects_completion_after_committed_cancellation(void) {
    cancellation_schedule("cancel-first", "outcome=010\noutcome=010\n");
}
/** Imported bytes remain uncommitted until the task boundary; cancellation there also clears its mailbox ownership. */
static void it_cancels_an_available_but_uncommitted_completion(void) {
    cancellation_schedule("available-before-cancel", "outcome=010\n");
}
/** A committed completion absorbs every later cancellation request without creating queue membership. */
static void it_preserves_completion_when_cancellation_is_requested_after_commit(void) {
    cancellation_schedule("completion-first", "outcome=001\noutcome=001\n");
}
/** Repeated readiness and premature resumes never fabricate completion or swallow cancellation. */
static void it_retains_cancellation_after_a_premature_resume(void) {
    cancellation_schedule("early-resume", "outcome=010\n");
}

/** The explicit cancellation cleanup method is harmless for an empty owner and after its first successful retirement. */
static void it_detaches_empty_and_armed_operations_idempotently(void) {
    cancellation_schedule("detach-idempotence", "");
}

/** A new enrollment resets its private loop predicate and outcome without reviving the previous native token. */
static void it_completes_a_fresh_run_after_cancelling_the_previous_enrollment(void) {
    cancellation_schedule("restart", "outcome=010\noutcome=001\n");
}

/** Run both terminal orderings and the explicit queue/wait boundary cases. */
void run(void) {
    it_commits_cold_cancellation_without_losing_the_initial_wake();
    it_wakes_once_and_rejects_completion_after_committed_cancellation();
    it_cancels_an_available_but_uncommitted_completion();
    it_preserves_completion_when_cancellation_is_requested_after_commit();
    it_retains_cancellation_after_a_premature_resume();
    it_detaches_empty_and_armed_operations_idempotently();
    it_completes_a_fresh_run_after_cancelling_the_previous_enrollment();
}
/** Return success only after all actual generated programs satisfy their outcome and cleanup assertions. */
int main(void) { run(); return EXIT_SUCCESS; }
