/**
 * @file
 * @brief Assemble recognized argv entries into the owned request configuration.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps argument
 * consumption independent of source loading and command execution.
 */
#ifndef AMA_DRIVER_OPTION_CONSUMPTION_H
#define AMA_DRIVER_OPTION_CONSUMPTION_H
#include "driver/options/storage/storage.h"

/** Consume argv[4..count), preserving exact arity and existing repeated options.
 * Allocated arrays each have capacity count. Strings remain borrowed. Failure
 * leaves a diagnostic and an invalid partial owner safe to destroy.
 */
bool driver_options_consume(DriverOptions *, DriverOptionStorage *, int count, char *const arguments[]);
#endif
