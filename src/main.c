#include "project/project.h"
#include <stdio.h>
#include <string.h>

/* CLI owns process exit and diagnostic presentation; compiler APIs return errors. */

#ifndef AMA_VERSION
#define AMA_VERSION "0.1.0"
#endif

static void print_usage(FILE* output, const char* program_name) {
    fprintf(output,
        "Ama compiler " AMA_VERSION "\n"
        "\n"
        "Usage: %s <root> <entry> <out.c>\n"
        "       %s --generate-c-stubs <root> <header-directory>\n"
        "\n"
        "Arguments:\n"
        "  <root>    project root directory (contains module files)\n"
        "  <entry>   entry module path relative to <root> (e.g., 'main')\n"
        "  <out.c>   path of the generated C file\n"
        "\n"
        "Example:\n"
        "  %s ./examples/hello main hello.c\n"
        "  gcc -O3 hello.c build/stdlib/libama_std.a -o hello\n"
        "  ./hello\n"
        "\n"
        "Options:\n"
        "  --generate-c-stubs  create missing native C stubs and generated headers\n"
        "  --generate-c-headers  generate headers without creating C sources\n"
        "  AMA_STDLIB  standard library directory (empty disables loading)\n"
        "  -h, --help     show this message and exit\n"
        "  -V, --version  show version and exit\n"
        "\n"
        "Exit status:\n"
        "  0  success\n"
        "  1  usage error or compilation error\n",
        program_name, program_name, program_name);
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
    bool success;
    if (!strcmp(arguments[1], "--generate-c-stubs"))
        success = generate_c_stubs(arguments[2], arguments[3], &error);
    else if (!strcmp(arguments[1], "--generate-c-headers"))
        success = generate_c_headers(arguments[2], arguments[3], &error);
    else
        success = compile_project(arguments[1], arguments[2], arguments[3], &error);
    if (!success) {
        error_print(stderr, &error);
        return 1;
    }
    return 0;
}
