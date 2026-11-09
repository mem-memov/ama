#include "../common.c"

static void it_compiles_unicode_module_paths_parameters_results_and_locals(void) {
    setup_test();
    const char* sources[] = {"project/\u5546\u5e97/\u5546\u54c1.c"};
    expect_native_program_output("expected.txt", sources, 1);
    destroy_test();
}

void run(void) {
    it_compiles_unicode_module_paths_parameters_results_and_locals();
}

int main(void) {
    run();
    printf("47-native-unicode: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
