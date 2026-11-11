#ifndef AMA_DRIVER_COMMANDS_H
#define AMA_DRIVER_COMMANDS_H

#include <stdio.h>
#include "driver/options/options.h"

int driver_emit(const DriverOptions *, FILE *, FILE *);
int driver_native(const DriverOptions *options, FILE *output, FILE *errors);
int driver_status(const DriverOptions *options, FILE *output, FILE *errors);
int driver_check(const DriverOptions *options, FILE *output, FILE *errors);

#endif
