#ifndef AMA_DRIVER_H
#define AMA_DRIVER_H

#include <stdio.h>

int driver_run(int argument_count, char *const arguments[],
    FILE *output, FILE *errors);

#endif
