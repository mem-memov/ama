#include "../common.c"

static void it_ignores_unfinished_interfaces_and_unselected_c_files(void) {
    setup_test();
    expect_program_output("expected.txt");
    destroy_test();
}

void run(void) {
    it_ignores_unfinished_interfaces_and_unselected_c_files();
}

int main(void) {
    run();
    printf("32-ignore-drafts: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
