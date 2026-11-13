/**
 * @file
 * @brief Execute native tools with explicit argv and checked child status.
 *
 * [STACK-04](../../../spec/implementation/compiler.md#stack-04)
 * No shell evaluates flags or paths. Compile a non-LTO object, inspect it, then link with explicit ordered arguments.
 */
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "internal.h"
#include "native/toolchain/profile.h"

/**
 * Fork/exec exact argv, optionally capture stdout and redirect diagnostics, then wait through
 * EINTR. Spawn/nonzero/signal failures record an IO diagnostic; no shell is invoked.
 */
bool codegen_run_tool(NativeScaffold *scaffold, char *const arguments[], const char *capture, FILE *errors) {
    pid_t process = fork();
    if (process < 0) { return scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot start the native toolchain", NULL); }
    if (process == 0) {
        if (errors != NULL && dup2(fileno(errors), STDERR_FILENO) < 0) { _exit(126); }
        if (capture != NULL && freopen(capture, "wb", stdout) == NULL) { _exit(126); }
        execvp(arguments[0], arguments);
        _exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(process, &status, 0); } while (waited < 0 && errno == EINTR);
    return (waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0)
        || scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "The native toolchain failed; no executable was published", arguments[0]);
}

/**
 * Allocate checked argv storage for caller-specified fixed tail capacity, copy borrowed
 * compiler/flags and rely on zeroed allocation for spare entries. Extra is a small internal
 * constant.
 */
static char **codegen_arguments(NativeScaffold *scaffold, size_t extra) {
    if (scaffold->options.flag_count > SIZE_MAX / sizeof(char *) - extra) {
        scaffold_error(scaffold, DIAGNOSTIC_OUT_OF_MEMORY, "Native command arguments exceed addressable storage", NULL);
        return NULL;
    }
    char **arguments = scaffold_allocate(scaffold, (scaffold->options.flag_count + extra) * sizeof(char *));
    if (arguments == NULL) { return NULL; }
    arguments[0] = (char *)(scaffold->options.compiler == NULL ? "gcc" : scaffold->options.compiler);
    for (size_t index = 0; index < scaffold->options.flag_count; index++) { arguments[index+1] = (char *)scaffold->options.flags[index]; }
    return arguments;
}

/**
 * Execute the configured compiler with explicit supported C11/non-LTO arguments and private
 * source/include/object paths; borrow error stream through child completion.
 */
bool codegen_compile_object(NativeScaffold *scaffold, const char *stage, FILE *errors) {
    const char *source = scaffold_join(scaffold, stage, "/", "launcher.c");
    const char *object = scaffold_join(scaffold, stage, "/", "program.o");
    const char *flags[] = {AMA_TOOLCHAIN_REQUIRED_FLAGS, "-O2", "-I", stage,
        "-I", scaffold->sdk, "-c", source, "-o", object, NULL};
    char **arguments = codegen_arguments(scaffold, 1 + sizeof(flags) / sizeof(flags[0]));
    if (arguments == NULL || source == NULL || object == NULL) { return false; }
    size_t index = 1 + scaffold->options.flag_count;
    for (size_t flag = 0; flag < sizeof(flags)/sizeof(flags[0]); flag++) { arguments[index++] = (char *)flags[flag]; }
    return codegen_run_tool(scaffold, arguments, NULL, errors);
}

/**
 * Link only the verified private object, placing user link flags after the object. Result
 * remains private until retained and explicitly published.
 */
bool codegen_link_object(NativeScaffold *scaffold, const char *stage, FILE *errors) {
    char **arguments = codegen_arguments(scaffold, 8);
    const char *object = scaffold_join(scaffold, stage, "/", "program.o");
    const char *program = scaffold_join(scaffold, stage, "/", "program");
    if (arguments == NULL || object == NULL || program == NULL) { return false; }
    memmove(arguments + 2, arguments + 1, scaffold->options.flag_count * sizeof(char *));
    arguments[1] = (char *)object;
    size_t index = 2 + scaffold->options.flag_count;
    const char *flags[] = {"-fno-lto", "-o", program, NULL};
    for (size_t flag = 0; flag < sizeof(flags)/sizeof(flags[0]); flag++) { arguments[index++] = (char *)flags[flag]; }
    return codegen_run_tool(scaffold, arguments, NULL, errors);
}
