#include "../common.c"

static void it_exports_headers_without_creating_missing_c_implementations(void) {
    setup_test();
    char* arguments[] = {AMA_COMPILER, "--generate-c-headers", "project", "generated/headers", NULL};
    int status = run_process(arguments, "generated/headers.out", "generated/headers.err");
    expect_status(status, 1, "generated/headers.err");
    assert(access("project/main.c", F_OK) != 0);
    destroy_test();
}

void run(void) {
    it_exports_headers_without_creating_missing_c_implementations();
}

int main(void) {
    run();
    printf("43-read-only-header-export: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
