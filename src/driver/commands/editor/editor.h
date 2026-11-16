/** @file
 * @brief Run explicit read-only editor snapshots and validated versioned rename plans through compiler-backed APIs.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02)
 * preserves one parser/type system; no document writes or native execution occur here.
 */
#ifndef AMA_DRIVER_EDITOR_H
#define AMA_DRIVER_EDITOR_H
#include <stdio.h>
#include "driver/options/options.h"
/** Borrow validated options/streams, retain source owners through output, flush success and release every partial owner. */
int driver_editor(const DriverOptions *, FILE *, FILE *);
#endif
