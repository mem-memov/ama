#ifndef AMA_TESTS_INTEGRATION_COMMON_C
#define AMA_TESTS_INTEGRATION_COMMON_C
#include "../common.c"
#include "process.c"

static void setup_test(void) {
    test_count++;
}

static void destroy_test(void) {
    /* The leaf Makefile removes generated/ after success and preserves it on failure. */
}

static inline void show_file(const char* path) {
    FILE* file = fopen(path, "rb");
    assert(file != NULL);
    int character;
    while ((character = fgetc(file)) != EOF) fputc(character, stderr);
    close_file(file);
}

static inline void expect_status(int actual, int expected, const char* errors) {
    if (actual != expected) {
        fprintf(stderr, "expected exit %d, got %d; see %s\n", expected, actual, errors);
        show_file(errors);
    }
    assert(actual == expected);
}

static inline int compile_ama(void) {
    char* arguments[] = {AMA_COMPILER, "project", "main", "generated/program.c", NULL};
    return run_process(arguments, "generated/ama.out", "generated/ama.err");
}

static inline void assert_files_equal(const char* actual_path, const char* expected_path) {
    FILE* actual = fopen(actual_path, "rb");
    FILE* expected = fopen(expected_path, "rb");
    assert(actual != NULL && expected != NULL);
    size_t offset = 0;
    for (;;) {
        int actual_character = fgetc(actual), expected_character = fgetc(expected);
        if (actual_character != expected_character)
            fprintf(stderr, "%s differs from %s at byte %zu: got %d, expected %d\n",
                actual_path, expected_path, offset, actual_character, expected_character);
        assert(actual_character == expected_character);
        if (actual_character == EOF) break;
        offset++;
    }
    assert(!ferror(actual) && !ferror(expected));
    close_file(actual);
    close_file(expected);
}

static inline int compile_generated_program(const char* const* native_sources, size_t source_count) {
    char* arguments[64] = {TEST_CC, "-std=c11", "-O2", "-Igenerated", "generated/program.c"};
    assert(source_count + 8 < sizeof(arguments) / sizeof(arguments[0]));
    size_t position = 5;
    for (size_t index = 0; index < source_count; index++)
        arguments[position++] = (char*)native_sources[index];
    arguments[position++] = "-o";
    arguments[position++] = "generated/program";
    arguments[position] = NULL;
    return run_process(arguments, "generated/cc.out", "generated/cc.err");
}

static inline void expect_native_program_output(
    const char* expected,
    const char* const* native_sources,
    size_t source_count
) {
    int status = compile_ama();
    expect_status(status, 0, "generated/ama.err");
    status = compile_generated_program(native_sources, source_count);
    expect_status(status, 0, "generated/cc.err");
    char* program_arguments[] = {"./generated/program", NULL};
    status = run_process(program_arguments, "generated/stdout", "generated/stderr");
    expect_status(status, 0, "generated/stderr");
    assert_files_equal("generated/stdout", expected);
}

static inline void expect_program_output(const char* expected) {
    expect_native_program_output(expected, NULL, 0);
}

static inline void expect_compile_error(const char* expected) {
    int status = compile_ama();
    expect_status(status, 1, "generated/ama.err");
    FILE* file = fopen(expected, "rb");
    assert(file != NULL);
    char expected_message[4096];
    char* read_line = fgets(expected_message, sizeof(expected_message), file);
    assert(read_line != NULL);
    expected_message[strcspn(expected_message, "\r\n")] = '\0';
    close_file(file);
    file = fopen("generated/ama.err", "rb");
    assert(file != NULL);
    char diagnostic[8192];
    size_t length = fread(diagnostic, 1, sizeof(diagnostic) - 1, file);
    assert(!ferror(file));
    diagnostic[length] = '\0';
    close_file(file);
    if (!strstr(diagnostic, expected_message)) fprintf(stderr, "missing '%s' in:\n%s", expected_message, diagnostic);
    assert(strstr(diagnostic, expected_message) != NULL);
}
#endif
