#ifndef AMA_DRIVER_OUTPUT_H
#define AMA_DRIVER_OUTPUT_H

#include <stdio.h>

#include "../request/request.h"

int driver_write_response(DriverRequest request, FILE *output);

#endif
