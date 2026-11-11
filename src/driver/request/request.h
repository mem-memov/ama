#ifndef AMA_DRIVER_REQUEST_H
#define AMA_DRIVER_REQUEST_H

typedef enum {
    DRIVER_REQUEST_HELP,
    DRIVER_REQUEST_CHECK,
    DRIVER_REQUEST_STATUS,
    DRIVER_REQUEST_UNAVAILABLE
} DriverRequest;

/* Borrow arguments; count is nonnegative, and a zero count permits NULL.
 * Recognize standalone --help or --check before operand validation.
 * Full option validation belongs to driver/options.
 * See docs/implementation/contracts/driver.md for the complete contract.
 */
DriverRequest driver_parse_request(int argument_count, char *const arguments[]);

#endif
