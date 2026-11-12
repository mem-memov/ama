/**
 * @file
 * @brief Compile native probes, inspect surviving entries and run only accepted binaries.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_NATIVE_COMMON_C
#define AMA_TEST_NATIVE_COMMON_C

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "native/toolchain/symbols.h"

/**
 * Run one exact native tool argv under the fixture output/diagnostic policy and return the
 * observed status for positive or negative assertions.
 */
static int run_tool(char *const arguments[], const char *output) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        int descriptor = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        int errors = open("generated/errors.log", O_WRONLY | O_CREAT | O_APPEND, 0600);
        if (descriptor < 0 || errors < 0 || dup2(descriptor, 1) < 0 || dup2(errors, 2) < 0) {
            _exit(127);
        }
        close(descriptor);
        close(errors);
        execvp(arguments[0], arguments);
        _exit(127);
    }
    int status;
    pid_t waited;
    do {
        waited = waitpid(process, &status, 0);
    } while (waited < 0 && errno == EINTR);
    assert(waited == process && WIFEXITED(status));
    return WEXITSTATUS(status);
}

/**
 * Compile the selected maintained C probe using the supported toolchain flags into test-owned
 * object storage; assert or return the expected outcome at its caller.
 */
static int compile_fixture(const char *source, const char *optimization) {
    char *arguments[] = {
        TEST_REAL_COMPILER, "-std=c11", "-pedantic-errors", "-Wall", "-Wextra", "-Werror",
        "-Werror=attributes", "-Werror=inline", "-fno-lto",
        "-fno-ipa-cp-clone", "-fno-ipa-sra", "-fno-optimize-sibling-calls",
        "-I" TEST_ROOT "/src", (char *) optimization, "-c", (char *) source,
        "-o", "generated/module.o", NULL
    };
    return run_tool(arguments, "generated/compiler.log");
}

/**
 * Inspect the actual probe object symbol listing and classify the exact native entry using the
 * production checker, not source spelling or execution alone.
 */
static AmaSymbolCheck inspect_entry(void) {
    char *arguments[] = {"nm", "--format=posix", "generated/module.o", NULL};
    int result = run_tool(arguments, "generated/symbols.txt");
    assert(result == 0);
    FILE *file = fopen("generated/symbols.txt", "rb");
    assert(file != NULL);
    unsigned char bytes[32768];
    size_t length = fread(bytes, 1, sizeof(bytes), file);
    assert(!ferror(file) && length < sizeof(bytes));
    int closed = fclose(file);
    assert(closed == 0);
    const unsigned char entry_name[] = "ama_fixture_native_entry";
    AmaByteView entry = {entry_name, sizeof(entry_name) - 1};
    return ama_check_native_symbols((AmaByteView) {bytes, length}, &entry, 1);
}

/**
 * Link and run the accepted probe only after object inspection; verify normal process success so
 * surviving-entry rejection cannot be bypassed.
 */
static void link_and_execute(void) {
    char *link[] = {TEST_REAL_COMPILER, "-fno-lto", "generated/module.o", "-o", "generated/program", NULL};
    int result = run_tool(link, "generated/link.log");
    assert(result == 0);
    char *execute[] = {"./generated/program", NULL};
    result = run_tool(execute, "generated/program.log");
    assert(result == 0);
}

#endif
