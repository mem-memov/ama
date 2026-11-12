/**
 * @file
 * @brief Check request operands and cross-option requirements without execution.
 *
 * [ARCH-02](../../../../spec/implementation/compiler.md#arch-02) separates
 * validation from storage and command effects; failures publish diagnostics.
 */
#ifndef AMA_DRIVER_OPTION_VALIDATION_H
#define AMA_DRIVER_OPTION_VALIDATION_H
#include "driver/options/options.h"

/** Record a borrowed program-lifetime diagnostic message and return false. */
bool driver_options_invalid(DriverOptions *, const char *message);

/** Reject missing operands or a path unsupported by the current path resolver.
 * The caller supplies the nonnegative valid argv domain of the request API.
 */
bool driver_options_operands(DriverOptions *, int count, char *const arguments[]);

/** Validate request-specific relations and choose the existing GCC default.
 * This checks presence/relations only; it does not create files or invoke tools.
 */
bool driver_options_validate(DriverOptions *);
#endif
