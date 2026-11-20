/** @file
 * @brief Run real check and native-authoring processes against isolated copies of invalid native definitions.
 */
#include <sys/stat.h>
#include "../common.c"
#include "../../unit/fixtures/package.h"

/** Copy maintained source bytes into test-owned scratch before any authoring
 * command; failed publication must never mutate maintained fixture files.
 */
static void prepare_project(void) {
    int result = mkdir("generated/project", 0700);
    assert(result == 0);
    fixture_package_prepare("generated/project", "fixture");
    const char *names[] = {"bad", "main", "plain"};
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        char source[128], destination[128];
        int length = snprintf(source, sizeof(source), "project/packages/test/fixture/v1.0.0/src/%s", names[index]);
        assert(length > 0 && (size_t)length < sizeof(source));
        length = snprintf(destination, sizeof(destination), "generated/project/packages/test/fixture/v1.0.0/src/%s", names[index]);
        assert(length > 0 && (size_t)length < sizeof(destination));
        FILE *input = fopen(source, "rb"), *copy = fopen(destination, "wb");
        assert(input != NULL && copy != NULL);
        unsigned char buffer[4096];
        size_t count;
        while ((count = fread(buffer, 1, sizeof(buffer), input)) != 0) { assert(fwrite(buffer, 1, count, copy) == count); }
        assert(!ferror(input));
        int input_closed = fclose(input), copy_closed = fclose(copy);
        assert(input_closed == 0 && copy_closed == 0);
    }
}

/** Require the policy diagnostic and both actual source contexts, not an ABI,
 * missing-companion or C compiler failure that merely happens to refuse output.
 */
static void native_error(void) {
    char text[8192];
    read_stream(errors, text, sizeof(text));
    assert(strstr(text, "A native @C module cannot declare own static parameters") != NULL);
    assert(strstr(text, "own static parameter introduction") != NULL);
    assert(strstr(text, ".type") != NULL && strstr(text, "@C") != NULL);
    read_stream(output, text, sizeof(text));
    assert(text[0] == 0);
}

/** A failed definition check must leave all potential native publication paths
 * absent, including ordinary companions beside the copied source.
 */
static void no_native_outputs(void) {
    const char *paths[] = {"generated/native output", "generated/project/packages/test/fixture/v1.0.0/src/bad.h",
        "generated/project/packages/test/fixture/v1.0.0/src/bad.c", "generated/project/packages/test/fixture/v1.0.0/src/bad.abi"};
    for (size_t index = 0; index < sizeof(paths) / sizeof(*paths); index++) {
        struct stat metadata;
        assert(stat(paths[index], &metadata) == -1 && errno == ENOENT);
    }
}

/** Actual applications binding every native formal do not alter definition
 * admission. Check-only command reaches neither C compilation nor publication.
 */
static void it_rejects_native_templates_in_check_mode_even_with_concrete_uses(void) {
    prepare_project();
    const char *entries[] = {"/test/fixture/v1.0.0/src/bad", "/test/fixture/v1.0.0/src/main"};
    for (size_t index = 0; index < 2; index++) {
        setup_test();
        char *arguments[] = {AMA_EXECUTABLE, "--check", "generated/project", (char *)entries[index], NULL};
        assert(run_compiler_arguments(arguments) == EXIT_FAILURE);
        native_error();
        no_native_outputs();
        destroy_test();
    }
}

/** Explicit create/overwrite flags cannot specialize a native source definition
 * or reach companion/header generation before the source policy is satisfied.
 */
static void it_refuses_native_authoring_before_creating_companions_or_headers(void) {
    for (size_t overwrite = 0; overwrite < 2; overwrite++) {
        setup_test();
        char *arguments[] = {AMA_EXECUTABLE, "--native", "generated/project", "/test/fixture/v1.0.0/src/bad", "--out", "generated/native output",
            "--sdk", TEST_SDK, "--create", "--protocols", "=", overwrite != 0 ? "--overwrite" : NULL, NULL};
        assert(run_compiler_arguments(arguments) == EXIT_FAILURE);
        native_error();
        no_native_outputs();
        destroy_test();
    }
}

/** Execute both actual CLI and side-effect observations explicitly. */
static void run(void) {
    it_rejects_native_templates_in_check_mode_even_with_concrete_uses();
    it_refuses_native_authoring_before_creating_companions_or_headers();
}

/** Return success only after all child statuses/diagnostics/publication checks. */
int main(void) { run(); return EXIT_SUCCESS; }
