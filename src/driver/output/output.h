/**
 * @file
 * @brief Declare complete response writing and flushing on borrowed streams.
 *
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01)
 * A partial write/flush failure is unsuccessful; this interface owns no stream lifetime.
 */
#ifndef AMA_DRIVER_OUTPUT_H
#define AMA_DRIVER_OUTPUT_H

#include <stdio.h>

#include "../request/request.h"

/* Accept a declared request and a valid borrowed stream; never close it.
 * Return success only after both writing and flushing succeed.
 * Output failure may leave partial output. See the driver contract.
 */
/**
 * Write selected static response and flush the borrowed stream. Partial output is possible on
 * failure; never close the stream.
 * Component contract: [COMP-01](../../../spec/implementation/compiler.md#comp-01). */
int driver_write_response(DriverRequest request, FILE *output);

#endif
