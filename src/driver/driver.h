/**
 * @file
 * @brief Declare explicit CLI command dispatch over borrowed arguments and streams.
 *
 * [COMP-01](../../spec/implementation/compiler.md#comp-01)
 * Do not retain arguments or close streams; return process status only after selected command completion.
 */
#ifndef AMA_DRIVER_H
#define AMA_DRIVER_H

#include <stdio.h>

/* Borrow the argument vector and streams; do not modify or retain arguments.
 * A zero argument count permits NULL. Help and successful explicit frontend checking can succeed.
 * See docs/implementation/contracts/driver.md for domains and failure effects.
 */
/**
 * Recognize one exact request, validate owned option arrays, invoke one command and destroy
 * options on every return. Borrow argv/streams without modification or closing.
 * Component contract: [COMP-01](../../spec/implementation/compiler.md#comp-01). */
int driver_run(int argument_count, char *const arguments[],
    FILE *output, FILE *errors);

#endif
