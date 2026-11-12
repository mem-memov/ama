/**
 * @file
 * @brief Initialize and dispose the argument owner's arena on every exit.
 *
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) requires explicit
 * lifetime boundaries: arrays are owned, while argv strings remain borrowed.
 */
#include "driver/options/options.h"

void driver_options_init(DriverOptions *options, SourceAllocator allocator) {
    *options = (DriverOptions){0};
    source_arena_init(&options->arena, allocator);
}

void driver_options_destroy(DriverOptions *options) {
    source_arena_destroy(&options->arena);
    *options = (DriverOptions){0};
}
