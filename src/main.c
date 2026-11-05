#include "project/project.h"
#include <stdio.h>
#include <string.h>

/* ============================================================================
    * Compiler entry point
    * ============================================================================
    *
    * Responsibilities:
    *   1. Parse command-line arguments.
    *   2. Handle --help and --version.
    *   3. Validate argument count.
    *   4. Delegate to compile_project().
    *
    * Design notes:
    *
    *   - No semantic validation here. Whether <root> is a directory, whether
    *     <entry> exists as a file, whether <out.c> is writable, whether the
    *     project is structurally valid — all of that is checked inside
    *     compile_project() and its helpers. Duplicating checks in main.c would
    *     produce two different error messages for the same condition.
    *
    *   - No argument "parsing" in the GNU sense (no --output=FILE, no --).
    *     The CLI is three positional arguments. If more flexibility is ever
    *     needed, it can be added later without breaking existing invocations.
    *
    *   - Exit codes: 0 for success, 1 for any failure. The compiler returns
    *     diagnostics to main; only the CLI prints them and exits.
    *     A future version may introduce distinct codes (e.g., 2 for
    *     compilation errors vs 1 for usage errors), but uniform 1 is the
    *     simplest contract that works with shell scripts and CI.
    *
    *   - --help and --version are handled BEFORE the argument-count check.
    *     This makes `ama --help` work without the three required arguments —
    *     the conventional behavior for well-behaved CLI tools.
    *
    *   - Usage on error goes to stderr; usage on --help goes to stdout.
    *     This is standard POSIX practice and lets `ama --help | less` work.
    *
    *   - argv[0] may be NULL on some systems (e.g., after execve with an
    *     empty argv). We fall back to "ama" in that case.
    *
    *   - No signal handlers. SIGINT during compilation produces the default
    *     behavior (process exit); a custom handler would add no value.
    */

#ifndef AMA_VERSION
#define AMA_VERSION "0.1.0"
#endif

static void print_usage(FILE* output, const char* program_name) {
    fprintf(output,
        "Ama compiler " AMA_VERSION "\n"
        "\n"
        "Usage: %s <root> <entry> <out.c>\n"
        "\n"
        "Arguments:\n"
        "  <root>    project root directory (contains module files)\n"
        "  <entry>   entry module path relative to <root> (e.g., 'main')\n"
        "  <out.c>   path of the generated C file\n"
        "\n"
        "Example:\n"
        "  %s ./examples/hello main hello.c\n"
        "  gcc -O3 -march=native hello.c -o hello\n"
        "  ./hello\n"
        "\n"
        "Options:\n"
        "  -h, --help     show this message and exit\n"
        "  -V, --version  show version and exit\n"
        "\n"
        "Exit status:\n"
        "  0  success\n"
        "  1  usage error or compilation error\n",
        program_name, program_name);
}

int main(int argument_count, char** arguments) {
    const char* program_name = (argument_count > 0 && arguments[0] != NULL) ? arguments[0] : "ama";

    /* --help / --version work without the three required arguments. */
    if (argument_count == 2) {
        if (strcmp(arguments[1], "-h") == 0 || strcmp(arguments[1], "--help") == 0) {
            print_usage(stdout, program_name);
            return 0;
        }
        if (strcmp(arguments[1], "-V") == 0 || strcmp(arguments[1], "--version") == 0) {
            printf("Ama compiler %s\n", AMA_VERSION);
            return 0;
        }
    }

    if (argument_count != 4) {
        print_usage(stderr, program_name);
        return 1;
    }

    Error error;
    if (!compile_project(arguments[1], arguments[2], arguments[3], &error)) {
        error_print(stderr, &error);
        return 1;
    }
    return 0;
}
