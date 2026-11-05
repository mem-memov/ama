#include "../common.c"

static void it_combines_module_calls_nested_control_and_early_return(void) {
    setup_test();
    expect_program_output("expected.txt");
    destroy_test();
}

void run(void) {
    it_combines_module_calls_nested_control_and_early_return();
}

int main(void) {
    run();
    printf("15-call-control: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
