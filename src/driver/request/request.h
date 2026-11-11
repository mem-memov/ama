#ifndef AMA_DRIVER_REQUEST_H
#define AMA_DRIVER_REQUEST_H

typedef enum {
    DRIVER_REQUEST_HELP,
    DRIVER_REQUEST_UNAVAILABLE
} DriverRequest;

/* Borrow arguments; count is nonnegative, and a zero count permits NULL.
 * Recognize only the exact standalone --help request; otherwise unavailable.
 * See docs/implementation/contracts/driver.md for the complete contract.
 */
DriverRequest driver_parse_request(int argument_count, char *const arguments[]);

#endif
