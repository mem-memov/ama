#ifndef AMA_DRIVER_OUTPUT_H
#define AMA_DRIVER_OUTPUT_H

#include <stdio.h>

#include "../request/request.h"

/* Accept a declared request and a valid borrowed stream; never close it.
 * Return success only after both writing and flushing succeed.
 * Output failure may leave partial output. See the driver contract.
 */
int driver_write_response(DriverRequest request, FILE *output);

#endif
