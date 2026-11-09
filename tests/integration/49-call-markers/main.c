#include "../common.c"

static void it_executes_explicit_calls_and_exits_only_the_nearest_loop(void) {
    setup_test();
    expect_program_output("expected.txt");
    destroy_test();
}

void run(void) {
    it_executes_explicit_calls_and_exits_only_the_nearest_loop();
}

int main(void) {
    run();
    printf("49-call-markers: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
