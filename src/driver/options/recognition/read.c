/**
 * @file
 * @brief Recognize CLI options by exact spelling and an explicit command set.
 *
 * [COMP-05](../../../../spec/implementation/compiler.md#comp-05) requires
 * deterministic compiler decisions. A fixed table and equality comparisons
 * avoid inferred aliases, filesystem-dependent choices or spelling heuristics.
 */
#include <stddef.h>
#include <string.h>
#include "option.h"

#define CHECK_REQUEST (1u << DRIVER_REQUEST_CHECK)
#define STATUS_REQUEST (1u << DRIVER_REQUEST_STATUS)
#define NATIVE_REQUEST (1u << DRIVER_REQUEST_NATIVE)
#define EMIT_REQUEST (1u << DRIVER_REQUEST_EMIT)
#define ALL_REQUESTS (CHECK_REQUEST | STATUS_REQUEST | NATIVE_REQUEST | EMIT_REQUEST)
#define GENERATING_REQUESTS (NATIVE_REQUEST | EMIT_REQUEST)

/** Every exact spelling occurs once; request masks encode the existing CLI. */
static const DriverOption options[] = {
    {"--root", DRIVER_OPTION_ROOT, true, ALL_REQUESTS},
    {"--standard-root", DRIVER_OPTION_STANDARD_ROOT, true, ALL_REQUESTS},
    {"--exclude", DRIVER_OPTION_EXCLUDE, true, ALL_REQUESTS},
    {"--out", DRIVER_OPTION_OUTPUT, true, GENERATING_REQUESTS},
    {"--sdk", DRIVER_OPTION_SDK, true, GENERATING_REQUESTS},
    {"--cc", DRIVER_OPTION_COMPILER, true, GENERATING_REQUESTS},
    {"--protocols", DRIVER_OPTION_PROTOCOLS, true, NATIVE_REQUEST},
    {"--c-flag", DRIVER_OPTION_C_FLAG, true, GENERATING_REQUESTS},
    {"--compile", DRIVER_OPTION_COMPILE, false, EMIT_REQUEST},
    {"--create", DRIVER_OPTION_CREATE, false, NATIVE_REQUEST},
    {"--overwrite", DRIVER_OPTION_OVERWRITE, false, GENERATING_REQUESTS}
};

const DriverOption *driver_option_recognize(DriverRequest request, const char *name) {
    if (request < DRIVER_REQUEST_CHECK || request > DRIVER_REQUEST_EMIT) {
        return NULL;
    }
    unsigned selected = 1u << request;
    for (size_t index = 0; index < sizeof(options) / sizeof(*options); index++) {
        if ((options[index].requests & selected) != 0 && strcmp(options[index].name, name) == 0) {
            return &options[index];
        }
    }
    return NULL;
}
