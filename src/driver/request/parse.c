/**
 * @file
 * @brief Recognize exact command spellings without guessing operands or abbreviations.
 *
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * Standalone help requires exactly two arguments; remaining command options receive their own complete validation.
 */
#include "request.h"

#include <string.h>

/**
 * Require nonnegative argc and valid argv entries when present. Match exact command strings;
 * only standalone --help succeeds without later operand validation.
 */
DriverRequest driver_parse_request(int argument_count, char *const arguments[]) {
    if (argument_count == 2 && strcmp(arguments[1], "--help") == 0) {
        return DRIVER_REQUEST_HELP;
    }
    if (argument_count >= 2 && strcmp(arguments[1], "--check") == 0) {
        return DRIVER_REQUEST_CHECK;
    }
    if (argument_count >= 2 && strcmp(arguments[1], "--status") == 0) {
        return DRIVER_REQUEST_STATUS;
    }
    if (argument_count >= 2 && strcmp(arguments[1], "--native") == 0) {
        return DRIVER_REQUEST_NATIVE;
    }
    if (argument_count >= 2 && strcmp(arguments[1], "--emit") == 0) {
        return DRIVER_REQUEST_EMIT;
    }
    if (argument_count >= 2 && strcmp(arguments[1], "--library-check") == 0) {
        return DRIVER_REQUEST_LIBRARY_CHECK;
    }
    if (argument_count >= 2 && strcmp(arguments[1], "--library-header") == 0) {
        return DRIVER_REQUEST_LIBRARY_HEADER;
    }
    if (argument_count >= 2 && strcmp(arguments[1], "--library-build") == 0) {
        return DRIVER_REQUEST_LIBRARY_BUILD;
    }
    return DRIVER_REQUEST_UNAVAILABLE;
}
