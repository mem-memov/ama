#include "output.h"

#include <stdlib.h>

int driver_write_response(DriverRequest request, FILE *output) {
    const char *message = request == DRIVER_REQUEST_HELP
        ? "Ama implementation foundation\n"
            "Usage: ama --help\n"
            "       ama --check ROOT .module [--root DIR] [--exclude PATH]\n"
            "       ama --status ROOT .module [--root DIR] [--exclude PATH]\n"
            "       ama --native ROOT .module --out DIR --sdk DIR [--create]\n"
            "       ama --emit ROOT .module --out DIR --sdk DIR [--compile]\n"
            "Checking validates source and typed plans without running C.\n"
            "Use --emit --compile to build a verified executable.\n"
        : "ama: expected an explicit command.\n"
            "Use --help for implementation status.\n";

    if (fputs(message, output) == EOF || fflush(output) == EOF) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
