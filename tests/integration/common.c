#ifndef AMA_TESTS_INTEGRATION_COMMON_C
#define AMA_TESTS_INTEGRATION_COMMON_C
#include "../common.c"
#include "process.c"

static void setup_test(void) {
    count++;
}

static void destroy_test(void) {
    /* The leaf Makefile removes generated/ after success and preserves it on failure. */
}

static inline void show_file(const char* path) {
    FILE* file = fopen(path, "rb");
    assert(file != NULL);
    int ch;
    while ((ch = fgetc(file)) != EOF) fputc(ch, stderr);
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
    char* args[] = {AMA_COMPILER, "project", "main", "generated/program.c", NULL};
    return run_process(args, "generated/ama.out", "generated/ama.err");
}

static inline void assert_files_equal(const char* actual_path, const char* expected_path) {
    FILE* actual = fopen(actual_path, "rb");
    FILE* expected = fopen(expected_path, "rb");
    assert(actual != NULL && expected != NULL);
    size_t offset = 0;
    for (;;) {
        int a = fgetc(actual), e = fgetc(expected);
        if (a != e)
            fprintf(stderr, "%s differs from %s at byte %zu: got %d, expected %d\n",
                    actual_path, expected_path, offset, a, e);
        assert(a == e);
        if (a == EOF) break;
        offset++;
    }
    assert(!ferror(actual) && !ferror(expected));
    close_file(actual);
    close_file(expected);
}

static inline void expect_program_output(const char* expected) {
    int status = compile_ama();
    expect_status(status, 0, "generated/ama.err");
    char* build[] = {TEST_CC, "-std=c11", "-O2", "generated/program.c", "-o", "generated/program", NULL};
    status = run_process(build, "generated/cc.out", "generated/cc.err");
    expect_status(status, 0, "generated/cc.err");
    char* run[] = {"./generated/program", NULL};
    status = run_process(run, "generated/stdout", "generated/stderr");
    expect_status(status, 0, "generated/stderr");
    assert_files_equal("generated/stdout", expected);
}

static inline void expect_compile_error(const char* expected) {
    int status = compile_ama();
    expect_status(status, 1, "generated/ama.err");
    FILE* file = fopen(expected, "rb");
    assert(file != NULL);
    char needle[4096];
    char* line = fgets(needle, sizeof(needle), file);
    assert(line != NULL);
    needle[strcspn(needle, "\r\n")] = '\0';
    close_file(file);
    file = fopen("generated/ama.err", "rb");
    assert(file != NULL);
    char diagnostic[8192];
    size_t length = fread(diagnostic, 1, sizeof(diagnostic) - 1, file);
    assert(!ferror(file));
    diagnostic[length] = '\0';
    close_file(file);
    if (!strstr(diagnostic, needle)) fprintf(stderr, "missing '%s' in:\n%s", needle, diagnostic);
    assert(strstr(diagnostic, needle) != NULL);
}
#endif
