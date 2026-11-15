/** @file
 * @brief Execute unique typed FIFO membership, observed and batch ordering, and explicit Ready requeue.
 * [ASYNC-07](../../../../spec/language/async-and-parallelism.md#async-07)
 * requires visible membership and deterministic dispatch for a fixed observed stream.
 */
#include "../execution/common.c"
#include "support/instantiate.c"
#include "support/trace.c"

/** Read maintained Ama templates exactly; generated outputs are never interpreted as source fixtures. */
static void fixture(const char *name, char source[16384]) {
    char path[256];
    int length = snprintf(path, sizeof(path), "project/%s", name);
    assert(length > 0 && (size_t)length < sizeof(path));
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    size_t count = fread(source, 1, 16383, input);
    assert(count < 16383 && !ferror(input));
    int closed = fclose(input);
    assert(closed == 0);
    source[count] = 0;
}

/** The finite template schema permits substitution only of its listed whole module-path lines. */
static const char *const modules[] = {
    "record", "task", "queue", "entry", "enqueue", "pop", "step", "dispatch", "registry", "observe", "import", "reconcile"
};

/** Instantiate two distinct nominal domains by exact declared path substitution, never a type cast. */
static void install_domain(const char *domain) {
    char directory_path[256];
    int length = snprintf(directory_path, sizeof(directory_path), "generated/project/%s", domain);
    assert(length > 0 && (size_t)length < sizeof(directory_path));
    directory(directory_path);
    for (size_t index = 0; index < sizeof(modules) / sizeof(*modules); index++) {
        char name[128], source[16384], destination[256];
        length = snprintf(name, sizeof(name), "kind/%s", modules[index]);
        assert(length > 0 && (size_t)length < sizeof(name));
        fixture(name, source);
        length = snprintf(destination, sizeof(destination), "%s/%s", directory_path, modules[index]);
        assert(length > 0 && (size_t)length < sizeof(destination));
        FILE *output = fopen(destination, "wb");
        assert(output != NULL);
        typed_schema_render(output, domain, source, modules, sizeof(modules) / sizeof(*modules));
        int closed = fclose(output);
        assert(closed == 0);
    }
}

/** Compile the actual task graph and compare every dispatched identity and per-task step count. */
static void scenario(const char *name, const char *expected, bool success) {
    directory("generated/project");
    install_domain("red");
    install_domain("blue");
    native_fixture("trace", "@<\nid\n..std.index\nqueued\n..task.flag\n@C\n");
    native_entry_override = typed_schema_trace;
    char source[16384];
    fixture(name, source);
    execute_project(source, expected, success);
    native_entry_override = NULL;
}

/** Repeated readiness never duplicates steps; Ready yields rejoin behind already accepted work. */
static void it_coalesces_readiness_and_requeues_ready_yields_at_the_tail(void) {
    scenario("coalesce", "1:1\n2:1\n3:1\n1:2\n2:2\n3:2\n1:3\n1:4\n", true);
}
/** Actual native callback acceptance order becomes FIFO order without sorting by completion time. */
static void it_preserves_the_observed_native_completion_stream_order(void) {
    scenario("ordered", "3:1\n1:1\n2:1\n3:2\n1:2\n2:2\n", true);
}
/** A whole unordered batch is reconciled in the registry's documented stable append order. */
static void it_orders_unordered_batch_readiness_by_stable_registration_order(void) {
    scenario("unordered", "1:1\n2:1\n3:1\n1:2\n2:2\n3:2\n", true);
}
/** Membership is shared across matching queues; distinct nominal domains use their own explicit dispatchers. */
static void it_coalesces_across_queues_and_executes_two_distinct_nominal_task_domains(void) {
    scenario("multiple", "1:1\n4:1\n1:2\n4:2\n", true);
}
/** Stale entries neither clear a new enrollment's queued flag nor execute a removed owner's task. */
static void it_discards_obsolete_epochs_and_dead_owners_without_touching_new_membership(void) {
    scenario("stale", "1:1\n1:2\n3:1\n3:2\n", true);
}
/** Deliberate bad membership proves that native task-step instrumentation rejects queued execution. */
static void it_detects_a_step_attempt_before_membership_has_been_cleared(void) {
    scenario("guard", "", false);
    expect_diagnostic("Task still queued");
}
/** Select one named group for diagnosis; the normal suite executes all queue contracts. */
void run(void) {
    const char *selected = getenv("AMA_READY_QUEUE_CASE");
    if (selected == NULL || strcmp(selected, "coalesce") == 0) { it_coalesces_readiness_and_requeues_ready_yields_at_the_tail(); }
    if (selected == NULL || strcmp(selected, "ordered") == 0) { it_preserves_the_observed_native_completion_stream_order(); }
    if (selected == NULL || strcmp(selected, "unordered") == 0) { it_orders_unordered_batch_readiness_by_stable_registration_order(); }
    if (selected == NULL || strcmp(selected, "multiple") == 0) { it_coalesces_across_queues_and_executes_two_distinct_nominal_task_domains(); }
    if (selected == NULL || strcmp(selected, "stale") == 0) { it_discards_obsolete_epochs_and_dead_owners_without_touching_new_membership(); }
    if (selected == NULL || strcmp(selected, "guard") == 0) { it_detects_a_step_attempt_before_membership_has_been_cleared(); }
    assert(selected == NULL || strcmp(selected, "coalesce") == 0 || strcmp(selected, "ordered") == 0
        || strcmp(selected, "unordered") == 0 || strcmp(selected, "multiple") == 0 || strcmp(selected, "stale") == 0 || strcmp(selected, "guard") == 0);

}
/** Exit after every queue contract has been observed through generated execution. */
int main(void) { run(); return 0; }
