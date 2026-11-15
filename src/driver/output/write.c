/**
 * @file
 * @brief Write explicit CLI help or unavailable-command guidance and verify final flush.
 *
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * Static text describes retained implementation; writing success does not imply success of an unavailable request.
 */
#include "output.h"

#include <stdlib.h>

/**
 * Write selected static response and flush the borrowed stream. Partial output is possible on
 * failure; never close the stream.
 */
int driver_write_response(DriverRequest request, FILE *output) {
    const char *message = request == DRIVER_REQUEST_HELP
        ? "Ama implementation foundation\n"
            "Usage: ama --help\n"
            "       ama --check ROOT .module [--root DIR] [--exclude PATH]\n"
            "       ama --status ROOT .module [--root DIR] [--exclude PATH]\n"
            "       ama --native ROOT .module --out DIR --sdk DIR [--create]\n"
            "       ama --emit ROOT .module --out DIR --sdk DIR [--compile]\n"
            "       ama --library-check ROOT CONFIG [--root DIR] [--exclude PATH]\n"
            "       ama --library-header ROOT CONFIG [--root DIR] [--exclude PATH]\n"
            "       ama --library-build ROOT CONFIG --out DIR --sdk DIR [--cc TOOL]\n"
            "Use --standard-root DIR to resolve double-dot library paths.\n"
            "Checking validates source and typed plans without running C.\n"
            "Use --emit --compile to build a verified executable.\n"
        : "ama: expected an explicit command.\n"
            "Use --help for implementation status.\n";

    if (fputs(message, output) == EOF || fflush(output) == EOF) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
