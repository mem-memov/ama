#include "../common.c"

static void it_rejects_colliding_generated_function_names(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_colliding_generated_function_names();
}

int main(void) {
    run();
    printf("35-reject-native-collision: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
