#include "../common.c"

static void it_compiles_an_explicit_empty_code_only_module(void) {
    setup_test();
    expect_program_output("expected.txt");
    destroy_test();
}

void run(void) {
    it_compiles_an_explicit_empty_code_only_module();
}

int main(void) {
    run();
    printf("22-code-only: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
