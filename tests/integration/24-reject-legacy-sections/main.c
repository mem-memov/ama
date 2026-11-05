#include "../common.c"

static void it_rejects_legacy_invisible_section_separators(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_legacy_invisible_section_separators();
}

int main(void) {
    run();
    printf("24-reject-legacy-sections: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
