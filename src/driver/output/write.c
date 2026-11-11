#include "output.h"

#include <stdlib.h>

int driver_write_response(DriverRequest request, FILE *output) {
    const char *message = request == DRIVER_REQUEST_HELP
        ? "Ama implementation foundation\n"
            "Usage: ama --help\n"
            "Compilation and checking are not implemented yet.\n"
        : "ama: compilation and checking are not implemented yet.\n"
            "Use --help for implementation status.\n";

    if (fputs(message, output) == EOF || fflush(output) == EOF) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
