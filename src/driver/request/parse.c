#include "request.h"

#include <string.h>

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
    return DRIVER_REQUEST_UNAVAILABLE;
}
