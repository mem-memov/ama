#include "../common.c"

static void it_cleans_successful_leaves_only_after_execution_and_fixture_destruction(void) {
    setup_test();
    int result = run_make("generated/project/tests/left", "all", NULL);
    assert(result == EXIT_SUCCESS);
    expect_contents("generated/project/generated/left", "setup\nrun\ndestroy\n");
    assert(!path_exists("generated/project/tests/left/generated"));
    assert(path_exists("generated/project/tests/left/main.c"));
    assert(path_exists("generated/project/tests/common.c"));
    destroy_test();
}

static void it_builds_without_execution_and_preserves_outputs_for_debugging(void) {
    setup_test();
    int result = run_make("generated/project/tests/left", "build", NULL);
    assert(result == EXIT_SUCCESS);
    assert(path_exists("generated/project/tests/left/generated/test"));
    assert(!path_exists("generated/project/generated/left"));
    result = run_make("generated/project/tests/left", "run", NULL);
    assert(result == EXIT_SUCCESS);
    assert(path_exists("generated/project/tests/left/generated/test"));
    expect_contents("generated/project/generated/left", "setup\nrun\ndestroy\n");
    destroy_test();
}

static void it_preserves_failed_assertion_artifacts_until_explicit_cleanup(void) {
    setup_test();
    int result = run_make("generated/project/tests/left", "all",
        "TEST_DEFINES=-DFORCE_FAILURE=1");
    assert(result != EXIT_SUCCESS);
    expect_failure_outputs();
    result = run_make("generated/project/tests/left", "clean", NULL);
    assert(result == EXIT_SUCCESS);
    assert(!path_exists("generated/project/tests/left/generated"));
    assert(path_exists("generated/project/tests/left/main.c"));
    destroy_test();
}

static void it_keeps_assertions_enabled_even_if_test_definitions_request_ndebug(void) {
    setup_test();
    int result = run_make("generated/project/tests/left", "all",
        "TEST_DEFINES=-DFORCE_FAILURE=1 -DNDEBUG");
    assert(result != EXIT_SUCCESS);
    expect_failure_outputs();
    destroy_test();
}

static void it_retains_compilation_failure_artifacts_without_running_the_test(void) {
    setup_test();
    write_file("generated/project/tests/left/main.c", "this is not valid C;\n");
    int result = run_make("generated/project/tests/left", "all", NULL);
    assert(result != EXIT_SUCCESS);
    assert(path_exists("generated/project/tests/left/generated/compile.configuration"));
    assert(!path_exists("generated/project/tests/left/generated/test"));
    assert(!path_exists("generated/project/generated/left"));
    destroy_test();
}

static void it_preserves_objects_after_a_link_failure_without_running_the_test(void) {
    setup_test();
    int result = run_make("generated/project/tests/left", "all",
        "LINKER_FLAGS=-lama_test_missing_library");
    assert(result != EXIT_SUCCESS);
    assert(path_exists("generated/project/tests/left/generated/objects/local/main.o"));
    assert(!path_exists("generated/project/tests/left/generated/test"));
    assert(!path_exists("generated/project/generated/left"));
    destroy_test();
}

static void it_runs_and_cleans_each_child_through_a_parallel_parent(void) {
    setup_test();
    int result = run_make("generated/project/tests", "all", NULL);
    assert(result == EXIT_SUCCESS);
    expect_contents("generated/project/generated/left", "setup\nrun\ndestroy\n");
    expect_contents("generated/project/generated/right", "setup\nrun\ndestroy\n");
    assert(!path_exists("generated/project/tests/left/generated"));
    assert(!path_exists("generated/project/tests/right/generated"));
    destroy_test();
}

static void it_propagates_parent_failure_and_preserves_each_executed_child(void) {
    setup_test();
    int result = run_make("generated/project/tests", "all",
        "TEST_DEFINES=-DFORCE_FAILURE=1");
    assert(result != EXIT_SUCCESS);
    const char *names[] = {"left", "right"};
    size_t executed = 0;
    for (size_t index = 0; index < 2; index++) {
        char observation[128];
        char artifact[128];
        int length = snprintf(observation, sizeof(observation),
            "generated/project/generated/%s", names[index]);
        assert(length > 0 && (size_t) length < sizeof(observation));
        length = snprintf(artifact, sizeof(artifact),
            "generated/project/tests/%s/generated/started", names[index]);
        assert(length > 0 && (size_t) length < sizeof(artifact));
        if (path_exists(observation)) {
            expect_contents(observation, "setup\nrun\n");
            assert(path_exists(artifact));
            executed++;
        }
    }
    assert(executed > 0);
    destroy_test();
}

static void it_recursively_cleans_children_without_deleting_handwritten_inputs(void) {
    setup_test();
    int result = run_make("generated/project/tests/left", "build", NULL);
    assert(result == EXIT_SUCCESS);
    result = run_make("generated/project/tests/right", "build", NULL);
    assert(result == EXIT_SUCCESS);
    result = run_make("generated/project/tests", "clean", NULL);
    assert(result == EXIT_SUCCESS);
    assert(!path_exists("generated/project/tests/left/generated"));
    assert(!path_exists("generated/project/tests/right/generated"));
    assert(path_exists("generated/project/tests/common.c"));
    assert(path_exists("generated/project/tests/left/main.c"));
    assert(path_exists("generated/project/tests/right/main.c"));
    destroy_test();
}

void run(void) {
    it_cleans_successful_leaves_only_after_execution_and_fixture_destruction();
    it_builds_without_execution_and_preserves_outputs_for_debugging();
    it_preserves_failed_assertion_artifacts_until_explicit_cleanup();
    it_keeps_assertions_enabled_even_if_test_definitions_request_ndebug();
    it_retains_compilation_failure_artifacts_without_running_the_test();
    it_preserves_objects_after_a_link_failure_without_running_the_test();
    it_runs_and_cleans_each_child_through_a_parallel_parent();
    it_propagates_parent_failure_and_preserves_each_executed_child();
    it_recursively_cleans_children_without_deleting_handwritten_inputs();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
