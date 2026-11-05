#include "../common.c"

static void it_never_creates_sources_in_an_imported_library(void) {
    setup_test();
    int configured = setenv("AMA_STDLIB", "library", 1);
    assert(configured == 0);
    char* arguments[] = {AMA_COMPILER, "--generate-c-stubs", "project", "generated/headers", NULL};
    int status = run_process(arguments, "generated/stubs.out", "generated/stubs.err");
    expect_status(status, 1, "generated/stubs.err");
    assert(access("library/sys/math/add.c", F_OK) != 0);
    assert(access("project/main.c", F_OK) != 0);
    destroy_test();
}

void run(void) {
    it_never_creates_sources_in_an_imported_library();
}

int main(void) {
    run();
    printf("44-library-stub-protection: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
