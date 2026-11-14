/**
 * @file
 * @brief Declare independent check, status, native-authoring and project-emission operations.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * Each synchronous command borrows validated options/streams and owns its complete temporary frontend/scaffold lifetime.
 */
#ifndef AMA_DRIVER_COMMANDS_H
#define AMA_DRIVER_COMMANDS_H

#include <stdio.h>
#include "driver/options/options.h"
#include "project/dependencies/project.h"

/** Render every parsed empty-body warning while project source storage is live; propagate output failures. */
bool driver_warnings(const Project *, FILE *);

/**
 * Complete strict frontend, stage project artifacts and optionally compile/verify before
 * publication. Destroy scaffold/frontend on all paths; acknowledged success requires stream
 * flush.
 */
int driver_emit(const DriverOptions *, FILE *, FILE *);
/**
 * Compose explicit authoring/checking and create/overwrite policy, then stage/publish native
 * artifacts. Always destroy scaffold before its borrowed frontend and render errors before
 * source destruction.
 */
int driver_native(const DriverOptions *options, FILE *output, FILE *errors);
/**
 * Own syntax-only inspection, report its activation kind and flush output. Destroy inspection
 * after rendering source-backed diagnostics.
 */
int driver_status(const DriverOptions *options, FILE *output, FILE *errors);
/**
 * Own/check/destroy a frontend; render failure while source spans remain live. Return success
 * only after checked output and flush.
 */
int driver_check(const DriverOptions *options, FILE *output, FILE *errors);

/** Resolve explicit library exports and write checked metadata without creating a launcher or artifact. */
int driver_library_check(const DriverOptions *, FILE *, FILE *);

#endif
