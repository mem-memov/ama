#include "../common.c"

static void it_exits_the_nearest_loop(void) {
    setup_test();
    expect_program_output("expected.txt");
    destroy_test();
}

void run(void) {
    it_exits_the_nearest_loop();
}

int main(void) {
    run();
    printf("05-break: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
