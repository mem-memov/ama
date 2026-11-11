#ifndef AMA_DRIVER_COMMANDS_H
#define AMA_DRIVER_COMMANDS_H

#include <stdio.h>
#include "driver/options/options.h"

int driver_check(const DriverOptions *options, FILE *output, FILE *errors);

#endif
