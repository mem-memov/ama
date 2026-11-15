/** @file
 * @brief Execute bounded explicit task/import rounds and a native wake without callback dispatch.
 * [ASYNC-07](../../../../spec/language/async-and-parallelism.md#async-07)
 * bounds both work sources, arbitrates typed queues and blocks only after readiness checks.
 */
#include "../execution/common.c"
#include "../ready-queues/support/instantiate.c"
#include "../ready-queues/support/trace.c"

/** Finite declared namespace substitutions shared by both nominal task domains. */
static const char *const modules[] = {
    "record", "task", "queue", "entry", "enqueue", "pop", "step", "dispatch",
    "registry", "observe", "import", "reconcile", "validate", "audit"
};
/** Load maintained source only; no generated C/SDK content is used as an input oracle. */
static void fixture(const char *path, char source[32768]) {
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    size_t count = fread(source, 1, 32767, input);
    assert(count < 32767 && !ferror(input));
    int closed = fclose(input);
    assert(closed == 0);
    source[count] = 0;
}
/** Reuse the tested queue operations, overriding only the explicitly selected wait/record components. */
static void install_domain(const char *domain, const char *task) {
    char directory_path[256];
    int length = snprintf(directory_path, sizeof(directory_path), "generated/project/%s", domain);
    assert(length > 0 && (size_t)length < sizeof(directory_path));
    directory(directory_path);
    for (size_t index = 0; index < sizeof(modules) / sizeof(*modules); index++) {
        const char *name = modules[index];
        bool local = strcmp(name, "record") == 0 || strcmp(name, "observe") == 0
            || strcmp(name, "validate") == 0 || strcmp(name, "audit") == 0 || strcmp(name, "task") == 0;
        char input[256], destination[256], source[32768];
        length = snprintf(input, sizeof(input), "%s/%s", local ? "project/kind" : "../ready-queues/project/kind",
            strcmp(name, "task") == 0 ? task : name);
        assert(length > 0 && (size_t)length < sizeof(input));
        fixture(input, source);
        length = snprintf(destination, sizeof(destination), "%s/%s", directory_path, name);
        assert(length > 0 && (size_t)length < sizeof(destination));
        FILE *output = fopen(destination, "wb");
        assert(output != NULL);
        typed_schema_render(output, domain, source, modules, sizeof(modules) / sizeof(*modules));
        int closed = fclose(output);
        assert(closed == 0);
    }
}
/** Install the authored native provider and data mailbox interfaces without interpreting generated artifacts. */
static void install_project(void) {
    directory("generated/project");
    install_domain("red", "loop-task");
    install_domain("blue", "wait-task");
    const char *names[] = {"source", "source.h", "source.c", "source.abi", "later", "later.h", "later.c", "later.abi", "round", "mailbox"};
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        char input[256], destination[256], source[32768];
        int length = snprintf(input, sizeof(input), "project/%s", names[index]);
        assert(length > 0 && (size_t)length < sizeof(input));
        fixture(input, source);
        length = snprintf(destination, sizeof(destination), "generated/project/%s", names[index]);
        assert(length > 0 && (size_t)length < sizeof(destination));
        file(destination, source);
    }
    native_fixture("trace", "@<\nid\n..std.index\nqueued\n..task.flag\n@C\n");
    native_fixture("summary", "@<\nsteps\n..integer\nimports\n..integer\ndecisions\n..integer\nready\n..task.flag\nwaiting\n..task.flag\nquiescent\n..task.flag\n@C\n");
}
/** Replace generated test stubs with the actual maintained provider bodies before compilation. */
static void authored_bodies(NativeScaffold *scaffold) {
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        const char *path = type->plan->type->module->source->logical_path;
        const char *name = strcmp(path, ".source") == 0 ? "source" : strcmp(path, ".later") == 0 ? "later" : NULL;
        if (name == NULL) { continue; }
        FILE *output = fopen(type->body_path, "wb");
        assert(output != NULL);
        int written = fprintf(output, "#include \"%s/tests/unit/codegen/scheduler-rounds/project/%s.c\"\n", TEST_ROOT, name);
        int closed = fclose(output);
        assert(written > 0 && closed == 0);
    }
}
/** Observe the actual published bounded metrics; task trace independently enforces cleared membership. */
static bool observations(FILE *output, const ScaffoldType *type) {
    if (typed_schema_trace(output, type)) { return true; }
    if (strcmp(type->plan->type->module->source->logical_path, ".summary") != 0) { return false; }
    int written = fputs("size_t steps,imports,decisions;\n"
        "*status=ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_to_index(parameters->AMA_INPUT(0)->payload,&steps); if(status->code!=AMA_OK) { return; }\n"
        "*status=ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_to_index(parameters->AMA_INPUT(1)->payload,&imports); if(status->code!=AMA_OK) { return; }\n"
        "*status=ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_to_index(parameters->AMA_INPUT(2)->payload,&decisions); if(status->code!=AMA_OK) { return; }\n"
        "printf(\"round=%zu/%zu/%zu:%u%u%u\\n\",steps,imports,decisions,(unsigned)parameters->AMA_INPUT(3)->payload->value,(unsigned)parameters->AMA_INPUT(4)->payload->value,(unsigned)parameters->AMA_INPUT(5)->payload->value);\n", output);
    assert(written != EOF);
    return true;
}
/** Enter a complete generated context with a real native provider, optional exact wait gate and owned schema. */
static void scenario(const char *name, const char *expected, bool gate, bool success) {
    fprintf(stderr, "scheduler scenario: %s\n", name);
    install_project();
    native_entry_override = observations;
    native_fixture_completion = authored_bodies;
    forced_header = gate ? TEST_ROOT "/tests/unit/codegen/scheduler-rounds/project/wait-hook.h" : NULL;
    char path[256], source[32768];
    int length = snprintf(path, sizeof(path), "project/%s", name);
    assert(length > 0 && (size_t)length < sizeof(path));
    fixture(path, source);
    execute_project(source, expected, success);
    forced_header = NULL;
    native_entry_override = NULL;
    native_fixture_completion = NULL;
}
/** Empty queues and no live waits report quiescence immediately instead of spinning through the configured quota. */
static void it_reports_quiescence_without_executing_or_blocking(void) {
    scenario("quiescent", "round=0/0/0:001\n", false, true);
}
/** A permanently Ready domain cannot starve another domain or its native completion import. */
static void it_bounds_replenished_ready_work_and_arbitrates_both_typed_queues(void) {
    scenario("fairness", "1:1\nround=1/0/1:100\n2:1\nround=1/1/1:100\n1:2\nround=1/0/1:100\n2:2\nround=1/0/1:100\nround=1/0/1:001\n", false, true);
}
/** Native backlog is imported at most once per selected quota, including unmatched detached records. */
static void it_bounds_native_collection_before_the_next_round(void) {
    scenario("backlog", "round=0/1/0:100\nround=0/1/0:100\nround=0/1/0:001\n", false, true);
}
/** Force a native completion after the last empty check; only the executor later imports and resumes the task. */
static void it_waits_without_losing_a_racing_native_completion(void) {
    scenario("blocking", "2:1\nround=1/0/1:010\nround=0/1/0:100\n2:2\nround=1/0/1:001\n", true, true);
}
/** Scheduling accepts only positive supported host-sized quotas before executing any task step. */
static void it_rejects_invalid_budgets_before_work_and_accepts_the_empty_maximum(void) {
    char maximum[3 * sizeof(size_t) + 2], overflow[sizeof(maximum) + 1];
    int maximum_length = snprintf(maximum, sizeof(maximum), "%zu", SIZE_MAX);
    assert(maximum_length > 0 && (size_t)maximum_length + 1 < sizeof(overflow));
    memcpy(overflow, maximum, (size_t)maximum_length);
    overflow[maximum_length] = '0';
    overflow[maximum_length + 1] = 0;
    const char *invalid[] = {"0", "-1", overflow};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        for (unsigned field = 0; field < 2; field++) {
            install_project();
            native_entry_override = observations;
            native_fixture_completion = authored_bodies;
            char setup[32768], source[65536];
            fixture("project/setup", setup);
            int length = snprintf(source, sizeof(source), "%sround !\n < &red registry\n < &blue registry\n < &red queue\n < &blue queue\n < &source\n < |%s\n < |%s\n",
                setup, field == 0 ? invalid[index] : "1", field == 1 ? invalid[index] : "1");
            assert(length > 0 && (size_t)length < sizeof(source));
            execute_project(source, "", false);
            expect_diagnostic("Scheduling budget must be positive and fit size_t");
            native_entry_override = NULL;
            native_fixture_completion = NULL;
        }
    }
    install_project();
    native_entry_override = observations;
    native_fixture_completion = authored_bodies;
    char setup[32768], source[65536];
    fixture("project/setup", setup);
    int length = snprintf(source, sizeof(source),
        "%sround !\n < &red registry\n < &blue registry\n < &red queue\n < &blue queue\n < &source\n < |%s\n < |%s\n"
        " > ready\n > waiting\n > quiescent\n > steps\n > imports\n > decisions\n"
        "summary !\n < steps\n < imports\n < decisions\n < ready\n < waiting\n < quiescent\n", setup, maximum, maximum);
    assert(length > 0 && (size_t)length < sizeof(source));
    execute_project(source, "round=0/0/0:001\n", true);
    native_entry_override = NULL;
    native_fixture_completion = NULL;
}
/** A live task missing its ready entry or live wait is a protocol error, never false quiescence. */
static void it_rejects_broken_live_task_states_before_blocking_or_quiescence(void) {
    scenario("missing-ready", "", false, false);
    expect_diagnostic("Task scheduler protocol invariant failed");
    scenario("broken-wait", "2:1\n", true, false);
    expect_diagnostic("Task scheduler protocol invariant failed");
}
/** Run all contracts by default; named groups make native wait diagnosis independent of elapsed-time guesses. */
void run(void) {
    const char *selected = getenv("AMA_SCHEDULER_CASE");
    if (selected == NULL || strcmp(selected, "quiescent") == 0) { it_reports_quiescence_without_executing_or_blocking(); }
    if (selected == NULL || strcmp(selected, "fairness") == 0) { it_bounds_replenished_ready_work_and_arbitrates_both_typed_queues(); }
    if (selected == NULL || strcmp(selected, "backlog") == 0) { it_bounds_native_collection_before_the_next_round(); }
    if (selected == NULL || strcmp(selected, "blocking") == 0) { it_waits_without_losing_a_racing_native_completion(); }
    if (selected == NULL || strcmp(selected, "budgets") == 0) { it_rejects_invalid_budgets_before_work_and_accepts_the_empty_maximum(); }
    if (selected == NULL || strcmp(selected, "protocol") == 0) { it_rejects_broken_live_task_states_before_blocking_or_quiescence(); }
    assert(selected == NULL || strcmp(selected, "quiescent") == 0 || strcmp(selected, "fairness") == 0
        || strcmp(selected, "backlog") == 0 || strcmp(selected, "blocking") == 0 || strcmp(selected, "budgets") == 0 || strcmp(selected, "protocol") == 0);
}
/** Exit after each actual execution and every captured bounded metric has matched its contract. */
int main(void) { run(); return 0; }
