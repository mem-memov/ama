#include "../common.c"

static void it_preserves_body_state_and_resets_call_parameters_and_results(void) {
    setup_test();
    expect_program_output("expected.txt");
    destroy_test();
}

void run(void) {
    it_preserves_body_state_and_resets_call_parameters_and_results();
}

int main(void) {
    run();
    printf("46-persistent-state: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
