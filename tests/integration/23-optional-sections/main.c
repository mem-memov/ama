#include "../common.c"

static void it_uses_result_defaults_with_omitted_sections(void) {
    setup_test();
    expect_program_output("expected.txt");
    destroy_test();
}

void run(void) {
    it_uses_result_defaults_with_omitted_sections();
}

int main(void) {
    run();
    printf("23-optional-sections: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
