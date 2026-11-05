#include "../common.c"

static void it_compiles_two_projects_in_one_process(void) {
    setup_test();
    bool first = compile_project(PROJECT_ROOT "/tests/integration/01-hello/project", "main", "generated/first.c", &error);
    bool second = compile_project(PROJECT_ROOT "/tests/integration/06-values/project", "main", "generated/second.c", &error);
    if (!second) error_print(stderr, &error);
    assert(first && second && error.error_code == ERROR_NONE);
    destroy_test();
}

static void it_can_compile_after_a_failed_compilation(void) {
    setup_test();
    bool rejected = compile_project(PROJECT_ROOT "/tests/integration/21-reject-cycle/project", "main", "generated/rejected.c", &error);
    assert(!rejected && error.error_code == ERROR_CYCLE);
    bool compiled = compile_project(PROJECT_ROOT "/tests/integration/01-hello/project", "main", "generated/recovered.c", &error);
    assert(compiled && error.error_code == ERROR_NONE);
    destroy_test();
}

static void it_returns_file_errors_to_its_caller(void) {
    setup_test();
    bool compiled = compile_project("generated/missing-input", "main", "generated/unused.c", &error);
    assert(!compiled && error.error_code == ERROR_IO);
    compiled = compile_project(PROJECT_ROOT "/tests/integration/01-hello/project", "main", "generated/missing-output/program.c", &error);
    assert(!compiled && error.error_code == ERROR_IO);
    destroy_test();
}

static void it_compiles_native_and_ama_projects_with_fresh_state(void) {
    setup_test();
    bool native = compile_project(PROJECT_ROOT "/tests/integration/26-native-calls/project",
        "main", "generated/native.c", &error);
    if (!native) error_print(stderr, &error);
    assert(native && error.error_code == ERROR_NONE);
    bool ama = compile_project(PROJECT_ROOT "/tests/integration/01-hello/project",
        "main", "generated/after-native.c", &error);
    assert(ama && error.error_code == ERROR_NONE);
    destroy_test();
}

void run(void) {
    it_compiles_native_and_ama_projects_with_fresh_state();
    it_compiles_two_projects_in_one_process();
    it_can_compile_after_a_failed_compilation();
    it_returns_file_errors_to_its_caller();
}

int main(void) {
    run();
    printf("unit/project/repeat: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
