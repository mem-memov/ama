#include "../common.c"

static void it_selects_the_true_branch(void) {
    setup_test();
    expect_program_output("expected.txt");
    destroy_test();
}

void run(void) {
    it_selects_the_true_branch();
}

int main(void) {
    run();
    printf("02-branch: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
