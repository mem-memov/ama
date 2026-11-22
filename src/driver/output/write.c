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
 * Component contract: [COMP-01](../../../spec/implementation/compiler.md#comp-01). */
int driver_write_response(DriverRequest request, FILE *output) {
    const char *message = request == DRIVER_REQUEST_HELP
        ? "Ama compiler and tools\n"
            "Usage: ama --help\n"
            "       ama --source-domain ROOT [--root DIR] [--standard-root DIR] [--exclude PATH]\n"
            "       ama --source-domain --snapshot FILE\n"
            "       ama --check ROOT MODULE [--root DIR] [--exclude PATH]\n"
            "       ama --status ROOT MODULE [--root DIR] [--exclude PATH]\n"
            "       ama --native ROOT MODULE --out DIR --sdk DIR [--create]\n"
            "       ama --emit ROOT MODULE --out DIR --sdk DIR --mode MODE --component ID [--compile]\n"
            "       ama --library-check ROOT CONFIG [--root DIR] [--exclude PATH]\n"
            "       ama --library-header ROOT CONFIG [--root DIR] [--exclude PATH]\n"
            "       ama --library-build ROOT CONFIG --out DIR --sdk DIR [--cc TOOL]\n"
            "       ama --editor ROOT ENTRY [--module SOURCE] [--offset BYTE]\n"
            "       ama --rename-plan ROOT ENTRY --offset BYTE --name NAME [--module SOURCE]\n"
            "       ama --relocate-preview ROOT S --destination D [--destination-root DIR]\n"
            "       ama --relocate-apply ROOT S --destination D --recovery DIR\n"
            "       ama --relocate-recover JOURNAL\n"
            "Relocation options: --entry MODULE --configuration FILE --invalidate PATH.\n"
            "Use --destination-standard-root DIR for an explicit standard destination root.\n"
            "Source tools write JSON; rename plans require an atomic editor document transaction.\n"
            "Use --standard-root DIR to resolve // standard-library paths.\n"
            "Checking validates source and typed plans without running C.\n"
            "Use --emit --compile to build a verified executable.\n"
        : "ama: expected an explicit command.\n"
            "Use --help for implementation status.\n";

    if (fputs(message, output) == EOF || fflush(output) == EOF) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
