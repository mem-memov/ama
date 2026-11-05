#include "../common.c"

static void it_checks_c_definitions_against_the_generated_header(void) {
    setup_test();
    int status = compile_ama();
    expect_status(status, 0, "generated/ama.err");
    const char* sources[] = {"project/main.c"};
    status = compile_generated_program(sources, 1);
    expect_status(status, 1, "generated/cc.err");
    destroy_test();
}

void run(void) {
    it_checks_c_definitions_against_the_generated_header();
}

int main(void) {
    run();
    printf("36-reject-native-signature: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
