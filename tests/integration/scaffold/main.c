/**
 * @file
 * @brief Exercise actual native authoring commands, Unicode paths and explicit overwrite policy.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#include <sys/stat.h>
#include "../common.c"
#include "../../unit/fixtures/package.h"

/**
 * Write a complete explicit fixture file and check final close; callers provide a test-owned
 * path and valid content.
 */
static void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "wb");
    assert(file != NULL);
    size_t written = fwrite(text, 1, strlen(text), file);
    int closed = fclose(file);
    assert(written == strlen(text) && closed == 0);
}

/**
 * Assert that the component creates unicode native companions and refreshes only owned outputs.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_creates_unicode_native_companions_and_refreshes_only_owned_outputs(void) {
    int created = mkdir("generated/project", 0700);
    assert(created == 0);
    fixture_package_prepare("generated/project", "fixture");
    write_file("generated/project/packages/test/fixture/v1.0.0/src/模块", "@C\n");
    setup_test();
    char *create[] = {AMA_EXECUTABLE, "--native", "generated/project", "/test/fixture/v1.0.0/src/模块", "--out", "generated/输出 files",
        "--sdk", TEST_SDK, "--create", "--protocols", "|=?[]", "--c-flag", "-DTEST_LABEL=\"two words\"", NULL};
    int result = run_compiler_arguments(create);
    char message[4096];
    read_stream(errors, message, sizeof(message));
    assert(result == EXIT_SUCCESS && message[0] == 0);
    destroy_test();
    setup_test();
    char *check[] = {AMA_EXECUTABLE, "--check", "generated/project", "/test/fixture/v1.0.0/src/模块", NULL};
    result = run_compiler_arguments(check);
    assert(result == EXIT_SUCCESS);
    destroy_test();
    setup_test();
    char *refresh[] = {AMA_EXECUTABLE, "--native", "generated/project", "/test/fixture/v1.0.0/src/模块", "--out", "generated/输出 files",
        "--sdk", TEST_SDK, "--c-flag", "-DTEST_LABEL=\"two words\"", NULL};
    result = run_compiler_arguments(refresh);
    assert(result == EXIT_SUCCESS);
    destroy_test();
    write_file("generated/输出 files/ama-native-types.h", "/* manually edited */\n");
    setup_test();
    result = run_compiler_arguments(refresh);
    read_stream(errors, message, sizeof(message));
    assert(result == EXIT_FAILURE && strstr(message, "conflicts") != NULL);
    destroy_test();
    setup_test();
    char *overwrite[] = {AMA_EXECUTABLE, "--native", "generated/project", "/test/fixture/v1.0.0/src/模块", "--out", "generated/输出 files",
        "--sdk", TEST_SDK, "--create", "--overwrite", "--c-flag", "-DTEST_LABEL=\"two words\"", NULL};
    result = run_compiler_arguments(overwrite);
    assert(result == EXIT_SUCCESS);
    destroy_test();
}

/**
 * Assert that the component checks exported commands without shell splitting unicode paths or
 * arguments. The explicit fixture setup and observations define this regression; assertion
 * failure keeps its generated evidence.
 */
static void it_checks_exported_commands_without_shell_splitting_unicode_paths_or_arguments(void) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execlp("python3", "python3", "-c",
            "import json,subprocess; "
            "db=json.load(open('generated/输出 files/compile_commands.json')); "
            "assert len(db)==3; "
            "assert all('-DTEST_LABEL=\"two words\"' in row['arguments'] for row in db); "
            "[subprocess.run(row['arguments'],cwd=row['directory'],check=True) for row in db]",
            (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/**
 * Assert that the component rejects invalid protocols and accidental overwrite requests. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_rejects_invalid_protocols_and_accidental_overwrite_requests(void) {
    setup_test();
    char *arguments[] = {AMA_EXECUTABLE, "--native", "generated/project", "/test/fixture/v1.0.0/src/模块", "--out", "generated/输出 files",
        "--sdk", TEST_SDK, "--create", "--protocols", "||", NULL};
    int result = run_compiler_arguments(arguments);
    char message[4096];
    read_stream(errors, message, sizeof(message));
    assert(result == EXIT_FAILURE && strstr(message, "unique optional symbols") != NULL);
    destroy_test();
    setup_test();
    char *overwrite[] = {AMA_EXECUTABLE, "--native", "generated/project", "/test/fixture/v1.0.0/src/模块", "--out", "generated/输出 files",
        "--sdk", TEST_SDK, "--overwrite", NULL};
    result = run_compiler_arguments(overwrite);
    assert(result == EXIT_FAILURE);
    destroy_test();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_creates_unicode_native_companions_and_refreshes_only_owned_outputs();
    it_checks_exported_commands_without_shell_splitting_unicode_paths_or_arguments();
    it_rejects_invalid_protocols_and_accidental_overwrite_requests();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
