#include "../common.c"

static void it_supplies_defaulted_result_pointers_on_every_call(void) {
    setup_test();
    const char* sources[] = {"project/value.c"};
    expect_native_program_output("expected.txt", sources, sizeof(sources) / sizeof(sources[0]));
    destroy_test();
}

void run(void) {
    it_supplies_defaulted_result_pointers_on_every_call();
}

int main(void) {
    run();
    printf("28-native-results: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
