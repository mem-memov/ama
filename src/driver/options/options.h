/**
 * @file
 * @brief Own one parsed CLI request while borrowing its argument strings.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) requires an explicit
 * boundary: this owner contains configuration and diagnostics, never a frontend.
 * [TOOL-04](../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps explicit captured inputs separate from physical root discovery.
 * [CLI-01](../../../spec/implementation/launch-and-embedding.md#cli-01) keeps
 * explicit component/mode configuration separate from runtime argument values.
 */
#ifndef AMA_DRIVER_OPTIONS_H
#define AMA_DRIVER_OPTIONS_H

#include "source/module/catalog.h"
#include "diagnostics/record/diagnostic.h"
#include "driver/request/request.h"
#include "delivery/mode.h"

/** Completed configuration; the arena owns arrays and argv owns stored strings. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
typedef struct {
    SourceArena arena;
    DriverRequest request;
    ModuleRoot *roots;
    size_t root_count;
    const char *entry;
    /** Explicit read-only captured input; roots remain absent in this mode under
     * [TOOL-04](../../../spec/implementation/tools-and-conformance.md#tool-04).
     */
    const char *source_snapshot;
    const char *query_module;
    const char *rename_name;
    size_t offset;
    bool has_offset;
    const char *relocation_source;
    const char *destination;
    ModuleRoot destination_root;
    const char *recovery_directory;
    const char *journal;
    const char **configurations;
    size_t configuration_count;
    const char **invalidations;
    size_t invalidation_count;
    const char *library_configuration;
    const char *output;
    const char *compiler;
    const char *sdk;
    const char **c_flags;
    size_t c_flag_count;
    /** Explicit compiler delivery authority under
     * [CLI-01](../../../spec/implementation/launch-and-embedding.md#cli-01).
     */
    const char *component;
    DeliveryMode delivery_mode;
    bool has_delivery_mode;
    bool compile;
    bool create;
    bool overwrite;
    const char *protocols;
    Diagnostic error;
    bool valid;
} DriverOptions;

/** Initialize an empty owner using the supplied allocation/release callbacks. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
void driver_options_init(DriverOptions *options, SourceAllocator allocator);
/** Release all complete/partial arena allocations and clear the owner. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
void driver_options_destroy(DriverOptions *options);
/** Parse once after initialization; borrow argv until destruction.
 * Exact options only, without implicit roots or inferred output exclusions.
 * Failure leaves valid false, a diagnostic and safely destructible partial arrays.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
bool driver_options_parse(DriverOptions *options, int count, char *const arguments[]);

#endif
