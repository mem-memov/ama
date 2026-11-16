/**
 * @file
 * @brief Bound and own CLI arrays, then publish their shared exclusion view.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) gives this
 * component storage ownership only; it never loads source or executes requests.
 */
#ifndef AMA_DRIVER_OPTION_STORAGE_H
#define AMA_DRIVER_OPTION_STORAGE_H
#include "driver/options/options.h"

/** Borrow the arena-owned exclusion array while one request is assembled. */
typedef struct {
    const char **exclusions;
    size_t excluded;
} DriverOptionStorage;

/** Allocate the three ordinary arrays and two optional relocation arrays, each bounded by capacity argument-derived entries.
 * On failure partial allocations remain owned by options until destruction.
 */
bool driver_options_allocate(DriverOptions *, size_t capacity, DriverOptionStorage *);

/** Attach the shared exclusion view to every root and mark assembly complete.
 * Requires all recognition and request validation to have succeeded.
 */
void driver_options_publish(DriverOptions *, const DriverOptionStorage *);
#endif
