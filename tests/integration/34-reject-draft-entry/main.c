#include "../common.c"

static void it_requires_an_implementation_for_the_entry_module(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_requires_an_implementation_for_the_entry_module();
}

int main(void) {
    run();
    printf("34-reject-draft-entry: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
