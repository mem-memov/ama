#include "request.h"

#include <string.h>

DriverRequest driver_parse_request(int argument_count, char *const arguments[]) {
    if (argument_count == 2 && strcmp(arguments[1], "--help") == 0) {
        return DRIVER_REQUEST_HELP;
    }
    return DRIVER_REQUEST_UNAVAILABLE;
}
