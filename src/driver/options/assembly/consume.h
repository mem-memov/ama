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
#include "driver/options/recognition/option.h"

/** Consume argv[4..count), preserving exact arity and existing repeated options.
 * Allocated arrays each have capacity count. Strings remain borrowed. Failure
 * leaves a diagnostic and an invalid partial owner safe to destroy.
 * Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
bool driver_options_consume(DriverOptions *, DriverOptionStorage *, int count, char *const arguments[]);
/** Install one source-tool scalar with duplicate/range checks before publishing options. Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
bool driver_options_source_tool(DriverOptions *, DriverOptionKind, const char *);
/** Install explicit relocation settings, rejecting duplicate scalar authority and retaining ordered configuration/output arrays. Component contract: [ARCH-03](../../../../spec/implementation/compiler.md#arch-03). */
bool driver_options_relocation(DriverOptions *, DriverOptionKind, const char *);
/** Install explicit mode/component once, preserving exact values under
 * [CLI-01](../../../../spec/implementation/launch-and-embedding.md#cli-01).
 */
bool driver_options_delivery(DriverOptions *, DriverOptionKind, const char *);
#endif
