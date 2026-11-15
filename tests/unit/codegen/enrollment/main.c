/** @file
 * @brief Execute compiler-visible task ownership and exact enrollment/wait identity checks.
 * [ASYNC-04](../../../../spec/language/async-and-parallelism.md#async-04)
 * requires typed owners/references, nonwrapping stamps and explicit invalidation.
 */
#include "../execution/common.c"

/** Load a maintained Ama acceptance input exactly; generated output is never read by this test. */
static void source_fixture(const char *name, char source[16384]) {
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

/** Install the same actual typed ownership graph for every independently compiled scenario. */
static void enrollment_fixture(void) {
    directory("generated/project");
    const char *names[] = {"record", "task", "registration", "validate"};
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        char source[16384], destination[256];
        source_fixture(names[index], source);
        int length = snprintf(destination, sizeof(destination), "generated/project/%s", names[index]);
        assert(length > 0 && (size_t)length < sizeof(destination));
        file(destination, source);
    }
}

/** The checked task stays physically owned and preserves its worker across restart; old stamps fail. */
static void it_invalidates_replaced_waits_and_old_runs_without_recreating_the_task(void) {
    enrollment_fixture();
    char source[16384];
    source_fixture("restart", source);
    execute_project(source, "1:1:0\n10:1:0\n20:2:0\n30:3:0\n40:4:0\n1:2:0\n50:5:0\n2:3:0\n60:6:0\n", true);
}

/** Removing only enrollment rejects old waits; removing ownership also invalidates the task handle. */
static void it_rejects_old_registrations_after_reenrollment_and_owner_reuse(void) {
    enrollment_fixture();
    char source[16384];
    source_fixture("removal", source);
    execute_project(source, "1:1:0\n10:1:0\n1:2:0\n20:2:0\n30:3:0\n1:1:0\n40:4:0\n50:5:0\n60:6:0\n70:7:8\n", true);
}

/** Ordinary entry/resume preserves explicit counters until the scheduler itself advances them. */
static void it_keeps_ordinary_calls_independent_of_scheduler_enrollment(void) {
    enrollment_fixture();
    char source[16384];
    source_fixture("ordinary", source);
    execute_project(source, "1:1:0\n10:1:0\n1:2:0\n20:2:0\n30:3:0\n40:4:0\n1:3:0\n50:5:0\n60:6:0\n2:4:0\n70:7:0\n", true);
}

/** Exhaustion is technical and prevents any further task step. */
static void it_reports_generation_exhaustion_before_further_task_execution(void) {
    const char *names[] = {"exhaust epoch", "exhaust wait"};
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        enrollment_fixture();
        char source[16384];
        source_fixture(names[index], source);
        execute_project(source, "1:1:0\n", false);
        expect_diagnostic("Task generation exhausted");
    }
}

/** Deliberately malformed native condition data exercises the flag's publication guard. */
static bool invalid_condition(FILE *output, const ScaffoldType *type) {
    if (strcmp(type->plan->type->module->source->logical_path, ".invalid") != 0) { return false; }
    int written = fputs("results->AMA_OUTPUT(0)->payload->value=2;\n", output);
    assert(written != EOF);
    return true;
}

/** A native producer cannot silently turn an out-of-range condition into scheduler membership. */
static void it_rejects_malformed_native_flag_inputs(void) {
    directory("generated/project");
    native_fixture("invalid", "@>\nresult\n..std.condition\n@C\n");
    native_entry_override = invalid_condition;
    execute_project("@!\nflag\n..task.flag\nproducer\n.invalid\nflag !\n < |1\n"
        "producer !\n > malformed\nflag !\n < malformed\n", "", false);
    expect_diagnostic("Invalid task flag");
    native_entry_override = NULL;
}

/** Register exact ownership and failure boundaries explicitly. */
void run(void) {
    it_invalidates_replaced_waits_and_old_runs_without_recreating_the_task();
    it_rejects_old_registrations_after_reenrollment_and_owner_reuse();
    it_keeps_ordinary_calls_independent_of_scheduler_enrollment();
    it_reports_generation_exhaustion_before_further_task_execution();
    it_rejects_malformed_native_flag_inputs();
}
/** Return only after actual generated label execution and checked references agree. */
int main(void) { run(); return EXIT_SUCCESS; }
