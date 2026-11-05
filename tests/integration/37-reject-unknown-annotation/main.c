#include "../common.c"

static void it_rejects_unimplemented_annotation_languages(void) {
    setup_test();
    expect_compile_error("expected-error.txt");
    destroy_test();
}

void run(void) {
    it_rejects_unimplemented_annotation_languages();
}

int main(void) {
    run();
    printf("37-reject-unknown-annotation: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
