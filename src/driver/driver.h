#ifndef AMA_DRIVER_H
#define AMA_DRIVER_H

#include <stdio.h>

/* Borrow the argument vector and streams; do not modify or retain arguments.
 * A zero argument count permits NULL. Help and successful explicit frontend checking can succeed.
 * See docs/implementation/contracts/driver.md for domains and failure effects.
 */
int driver_run(int argument_count, char *const arguments[],
    FILE *output, FILE *errors);

#endif
