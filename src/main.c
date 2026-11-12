/**
 * @file
 * @brief Adapt the process argument vector and standard streams to the explicit CLI driver.
 *
 * [COMP-01](../spec/implementation/compiler.md#comp-01)
 * This executable-only adapter owns no frontend or output state; driver return status becomes process exit status.
 */
#include "driver/driver.h"

/**
 * Borrow the process argv and standard streams and return driver status unchanged; the driver
 * owns all temporary state.
 */
int main(int argument_count, char **arguments) {
    return driver_run(argument_count, arguments, stdout, stderr);
}
