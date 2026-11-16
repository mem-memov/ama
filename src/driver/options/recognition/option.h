/**
 * @file
 * @brief Define exact option spellings, arity and command applicability.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps lexical
 * recognition independent of argument allocation and frontend state.
 */
#ifndef AMA_DRIVER_OPTION_RECOGNITION_H
#define AMA_DRIVER_OPTION_RECOGNITION_H
#include <stdbool.h>
#include "driver/request/request.h"

/** Closed option destinations; these are compiler CLI records, not Ama values. */
typedef enum {
    DRIVER_OPTION_ROOT, DRIVER_OPTION_EXCLUDE, DRIVER_OPTION_OUTPUT,
    DRIVER_OPTION_SDK, DRIVER_OPTION_COMPILER, DRIVER_OPTION_PROTOCOLS,
    DRIVER_OPTION_C_FLAG, DRIVER_OPTION_COMPILE, DRIVER_OPTION_CREATE,
    DRIVER_OPTION_OVERWRITE, DRIVER_OPTION_STANDARD_ROOT,
    DRIVER_OPTION_QUERY_MODULE, DRIVER_OPTION_OFFSET, DRIVER_OPTION_RENAME_NAME
} DriverOptionKind;

/** Borrowed immutable recognition result; a value consumes the next argv entry. */
typedef struct {
    const char *name;
    DriverOptionKind kind;
    bool takes_value;
    unsigned requests;
} DriverOption;

/** Return the exact applicable option, or NULL for unknown/inapplicable names.
 * Name is a borrowed nonnull C string; no allocation, prefix matching or mutation.
 */
const DriverOption *driver_option_recognize(DriverRequest, const char *name);
#endif
