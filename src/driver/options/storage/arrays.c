/**
 * @file
 * @brief Reserve bounded CLI storage and publish the completed root configuration.
 *
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) requires exact
 * bounds: argv entry count bounds every destination array, and checked products
 * precede allocation. No guessed capacity or root exclusion is introduced.
 */
#include <stdint.h>
#include <string.h>
#include "storage.h"
#include "driver/options/validation/validation.h"

bool driver_options_allocate(DriverOptions *options, size_t capacity, DriverOptionStorage *storage) {
    *storage = (DriverOptionStorage){0};
    if (capacity > SIZE_MAX / sizeof(ModuleRoot) || capacity > SIZE_MAX / sizeof(char *)) {
        return driver_options_invalid(options, "Command argument storage is unrepresentable");
    }
    options->roots = source_arena_allocate(&options->arena, capacity * sizeof(*options->roots));
    options->c_flags = source_arena_allocate(&options->arena, capacity * sizeof(*options->c_flags));
    storage->exclusions = source_arena_allocate(&options->arena, capacity * sizeof(*storage->exclusions));
    if (options->roots == NULL || options->c_flags == NULL || storage->exclusions == NULL) {
        options->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
            .message = "Command options could not allocate storage"};
        return false;
    }
    memset(options->roots, 0, capacity * sizeof(*options->roots));
    return true;
}

void driver_options_publish(DriverOptions *options, const DriverOptionStorage *storage) {
    for (size_t index = 0; index < options->root_count; index++) {
        options->roots[index].excluded = storage->exclusions;
        options->roots[index].excluded_count = storage->excluded;
    }
    options->valid = true;
}
