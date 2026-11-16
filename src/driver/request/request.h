/**
 * @file
 * @brief Declare exact command recognition independently of option validation.
 *
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * Borrow a valid nonnegative-count argv; count zero permits null and unknown commands remain unavailable.
 */
#ifndef AMA_DRIVER_REQUEST_H
#define AMA_DRIVER_REQUEST_H

/**
 * Exact supported CLI requests plus unavailable; recognizing a command does not validate its
 * operands or authorize implicit execution.
 * Component contract: [COMP-01](../../../spec/implementation/compiler.md#comp-01). */
typedef enum {
    DRIVER_REQUEST_HELP,
    DRIVER_REQUEST_CHECK,
    DRIVER_REQUEST_STATUS,
    DRIVER_REQUEST_NATIVE,
    DRIVER_REQUEST_EMIT,
    DRIVER_REQUEST_LIBRARY_CHECK,
    DRIVER_REQUEST_LIBRARY_HEADER,
    DRIVER_REQUEST_LIBRARY_BUILD,
    DRIVER_REQUEST_EDITOR,
    DRIVER_REQUEST_RENAME_PLAN,
    DRIVER_REQUEST_RELOCATE_PREVIEW,
    DRIVER_REQUEST_RELOCATE_APPLY,
    DRIVER_REQUEST_RELOCATE_RECOVER,
    DRIVER_REQUEST_UNAVAILABLE
} DriverRequest;

/* Borrow arguments; count is nonnegative, and a zero count permits NULL.
 * Recognize standalone --help or an exact checking, authoring, emission or library command before operand validation.
 * Full option validation belongs to driver/options.
 * See docs/implementation/contracts/driver.md for the complete contract.
 */
/**
 * Require nonnegative argc and valid argv entries when present. Match exact command strings;
 * only standalone --help succeeds without later operand validation.
 * Component contract: [COMP-01](../../../spec/implementation/compiler.md#comp-01). */
DriverRequest driver_parse_request(int argument_count, char *const arguments[]);

#endif
