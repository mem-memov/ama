#include "../common.c"

static void it_exports_typed_native_signatures_and_executes_their_adapters(void) {
    setup_test();
    char* exporter[] = {AMA_COMPILER, "--generate-type-layouts", "project", "generated/types", NULL};
    expect_status(run_process(exporter, "generated/export.out", "generated/export.err"), 0, "generated/export.err");
    char* compiler[] = {TEST_CC, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
        "-Igenerated/types", "-I" TEST_ROOT "/build/stdlib/include", "generated/types/ama_types.c",
        "project/native/buffer.c", "project/native/echo.c", "project/native/ping.c", "project/driver.c",
        AMA_STANDARD_LIBRARY_ARCHIVE, "-o", "generated/program", NULL};
    expect_status(run_process(compiler, "generated/cc.out", "generated/cc.err"), 0, "generated/cc.err");
    char* program[] = {"./generated/program", NULL};
    expect_status(run_process(program, "generated/stdout", "generated/stderr"), 0, "generated/stderr");
    assert_files_equal("generated/stdout", "expected.txt");
    destroy_test();
}

static void it_rejects_native_definitions_that_disagree_with_the_generated_signature(void) {
    setup_test();
    char* exporter[] = {AMA_COMPILER, "--generate-type-layouts", "project", "generated/types", NULL};
    expect_status(run_process(exporter, "generated/export.out", "generated/export.err"), 0, "generated/export.err");
    char* compiler[] = {TEST_CC, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic",
        "-Igenerated/types", "-I" TEST_ROOT "/build/stdlib/include", "-c", "project/invalid_signature.c",
        "-o", "generated/invalid.o", NULL};
    expect_status(run_process(compiler, "generated/invalid.out", "generated/invalid.err"), 1, "generated/invalid.err");
    destroy_test();
}

void run(void) {
    it_exports_typed_native_signatures_and_executes_their_adapters();
    it_rejects_native_definitions_that_disagree_with_the_generated_signature();
}

int main(void) {
    run();
    printf("58-typed-native-calls: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
