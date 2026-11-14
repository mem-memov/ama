/**
 * @file
 * @brief Validate command operands and native/emission option relations.
 *
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) requires exact
 * predicates. This preserves the existing command contract and delegates logical
 * path spelling to its source component; current path-model adaptation is T014.
 */
#include "validation.h"

bool driver_options_invalid(DriverOptions *options, const char *message) {
    options->error = (Diagnostic){.code = DIAGNOSTIC_INVALID_PATH, .message = message};
    return false;
}

bool driver_options_operands(DriverOptions *options, int count, char *const arguments[]) {
    if (options->request == DRIVER_REQUEST_LIBRARY_CHECK) {
        return count >= 4 && arguments[3][0] != '\0'
            ? true : driver_options_invalid(options, "Expected a source root and a library configuration path");
    }
    if (options->request == DRIVER_REQUEST_UNAVAILABLE || count < 4 || !module_path_valid(arguments[3])) {
        return driver_options_invalid(options, "Expected a source root and an exact logical module path");
    }
    return true;
}

bool driver_options_validate(DriverOptions *options) {
    if (options->request == DRIVER_REQUEST_NATIVE && (options->output == NULL || options->sdk == NULL
            || (options->overwrite && !options->create) || (options->protocols != NULL && !options->create))) {
        return driver_options_invalid(options, "Native generation needs --out/--sdk; --overwrite/--protocols require --create");
    }
    if (options->request == DRIVER_REQUEST_EMIT && (options->output == NULL || options->sdk == NULL)) {
        return driver_options_invalid(options, "Project emission needs --out and --sdk");
    }
    if (options->compiler == NULL) {
        options->compiler = "gcc";
    }
    return true;
}
