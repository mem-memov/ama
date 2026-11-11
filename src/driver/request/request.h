#ifndef AMA_DRIVER_REQUEST_H
#define AMA_DRIVER_REQUEST_H

typedef enum {
    DRIVER_REQUEST_HELP,
    DRIVER_REQUEST_UNAVAILABLE
} DriverRequest;

DriverRequest driver_parse_request(int argument_count, char *const arguments[]);

#endif
