/** @file
 * @brief Execute registration restart and owner retirement through actual standard-library views and stable pools.
 */
#include "../execution/common.c"

/** Load maintained Ama scenarios as test inputs; generated artifacts are never expected-output authorities. */
static void execute_detachment(const char *scenario) {
    char path[160];
    int length = snprintf(path, sizeof(path), "project/%s", scenario);
    assert(length > 0 && (size_t)length < sizeof(path));
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    char source[8192];
    size_t count = fread(source, 1, sizeof(source) - 1, input);
    assert(!ferror(input) && feof(input));
    source[count] = '\0';
    int closed = fclose(input);
    assert(closed == 0);
    if (strcmp(scenario, "suspended") == 0) {
        FILE *task = fopen("project/task", "rb");
        assert(task != NULL);
        char text[4096];
        size_t size = fread(text, 1, sizeof(text) - 1, task);
        assert(!ferror(task) && feof(task));
        text[size] = '\0';
        closed = fclose(task);
        assert(closed == 0);
        directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
        file("generated/project/packages/test/fixture/v1.0.0/src/task", text);
    }
    fprintf(stderr, "detachment scenario: %s\n", scenario);
    execute_project(source, "", true);
    expect_diagnostic("");
}

/** Restart frees capacity and makes late completion of the previous full token harmless. */
static void it_detaches_the_previous_operation_before_restarting(void) { execute_detachment("restart"); }

/** Removing an owning list element removes even queued completion bytes before the pool slot can be reused. */
static void it_retires_owned_operations_before_reusing_list_storage(void) { execute_detachment("retire"); }

/** Closing and reconstructing the source does not prevent retirement of an older retained endpoint. */
static void it_retires_operations_after_their_source_was_reconstructed(void) { execute_detachment("source-restart"); }

/** Removing a suspended owner invalidates its continuation while late callbacks cannot reach its replacement. */
static void it_retires_suspended_tasks_without_executing_their_continuations(void) { execute_detachment("suspended"); }

/** Run every actual native ownership boundary with a one-cell service to expose leaked reservations. */
void run(void) {
    it_detaches_the_previous_operation_before_restarting();
    it_retires_operations_after_their_source_was_reconstructed();
    it_retires_owned_operations_before_reusing_list_storage();
    it_retires_suspended_tasks_without_executing_their_continuations();
}

/** Report success only after both compiled lifecycle scenarios complete. */
int main(void) { run(); return EXIT_SUCCESS; }
