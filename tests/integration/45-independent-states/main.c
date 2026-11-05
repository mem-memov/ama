#include "../common.c"

static void it_runs_one_module_over_two_independent_states(void) {
    setup_test();
    int configured = setenv("AMA_STDLIB", "", 1);
    assert(configured == 0);
    int status = compile_ama();
    expect_status(status, 0, "generated/ama.err");
    char* compile_arguments[] = {TEST_CC, "-std=c11", "-Wall", "-Wextra", "-pedantic",
        "-Igenerated", "driver.c", "project/step.c", "-o", "generated/program", NULL};
    status = run_process(compile_arguments, "generated/cc.out", "generated/cc.err");
    expect_status(status, 0, "generated/cc.err");
    char* program_arguments[] = {"./generated/program", NULL};
    status = run_process(program_arguments, "generated/stdout", "generated/stderr");
    expect_status(status, 0, "generated/stderr");
    assert_files_equal("generated/stdout", "expected.txt");
    destroy_test();
}

void run(void) {
    it_runs_one_module_over_two_independent_states();
}

int main(void) {
    run();
    printf("45-independent-states: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
