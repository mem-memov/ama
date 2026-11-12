/**
 * @file
 * @brief Verify that recursive same-command checks cannot remove each other's candidates.
 *
 * This isolates the manifest overlap possible when parallel test leaves revisit
 * an already built compiler. It does not promise concurrent conflicting builds.
 * Its acceptance argument and specification links live in the incremental-build
 * contract; the fixture itself owns no production compiler state.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

#include "../../support/files.c"
#include "support/process.c"

/**
 * Own an isolated fixture and publish its initial manifest with real Make rules.
 * Assert setup/child failure; retained artifacts then remain available. No real
 * compiler is needed because this leaf exercises the command-manifest boundary.
 */
static void prepare_fixture(void) {
    make_directory("generated/project");
    copy_file(TEST_ROOT "/make/configuration.mk", "generated/project/configuration.mk");
    write_file("generated/project/Makefile",
        ".RECIPEPREFIX := >\nCONFIGURATION_DIRECTORY := cache\n"
        "COMPILE_COMMAND := cc -std=c11\nLINK_COMMAND := cc\n"
        "BINARY := unused\nOBJECTS :=\ninclude configuration.mk\n");
    pid_t process = fork();
    if (process == 0) {
        execute_check(NULL, 0, NULL);
    }
    int status = finish_check(process);
    assert(status == EXIT_SUCCESS);
}

/**
 * Start two checks whose recipes cannot run until the parent releases them.
 * @param[out] checks Receives owned child PIDs and parent pipe ends; both ready
 * and release pairs are initialized before any fork so child closure is exact.
 * The absolute shell option is copied by fork and borrowed until child exec.
 */
static void start_checks(ConfigurationCheck checks[2]) {
    char directory[4096];
    char *working_directory = getcwd(directory, sizeof(directory));
    assert(working_directory != NULL);
    char option[4352];
    int length = snprintf(option, sizeof(option), "SHELL=%s/generated/shell", directory);
    assert(length > 0 && (size_t)length < sizeof(option));
    for (size_t index = 0; index < 2; index++) {
        int ready = pipe(checks[index].ready);
        int release = pipe(checks[index].release);
        assert(ready == 0 && release == 0);
    }
    for (size_t index = 0; index < 2; index++) {
        checks[index].process = fork();
        if (checks[index].process == 0) {
            execute_check(checks, index, option);
        }
        assert(checks[index].process > 0);
    }
    for (size_t index = 0; index < 2; index++) {
        int ready = close(checks[index].ready[1]);
        int release = close(checks[index].release[0]);
        assert(ready == 0 && release == 0);
    }
}

/** Both candidates must survive the other recipe's successful cleanup. */
static void it_preserves_overlapping_same_configuration_checks(void) {
    prepare_fixture();
    ConfigurationCheck checks[2];
    start_checks(checks);
    expect_ready(checks[0].ready[0]);
    expect_ready(checks[1].ready[0]);
    release_check(&checks[0], 0);
    release_check(&checks[1], 1);
    remove_tree("generated/project");
}

/** A malformed manifest location must fail, not be treated as changed content. */
static void it_propagates_comparison_errors_without_replacing_the_old_location(void) {
    prepare_fixture();
    const char *path = "generated/project/cache/compile.configuration";
    int removed = unlink(path);
    assert(removed == 0);
    make_directory(path);
    pid_t process = fork();
    if (process == 0) {
        execute_check(NULL, 0, NULL);
    }
    int status = finish_check(process);
    assert(status != EXIT_SUCCESS);
    struct stat information;
    int observed = lstat(path, &information);
    assert(observed == 0 && S_ISDIR(information.st_mode));
    remove_tree("generated/project");
}

/** Run the controlled-interleaving and I/O-failure contracts explicitly. */
void run(void) {
    struct rlimit limit = {0, 0};
    int limited = setrlimit(RLIMIT_CORE, &limit);
    assert(limited == 0);
    it_preserves_overlapping_same_configuration_checks();
    it_propagates_comparison_errors_without_replacing_the_old_location();
}

/** Enter the independent C/assert test leaf. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
