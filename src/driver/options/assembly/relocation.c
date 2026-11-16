/** @file
 * @brief Retain explicit relocation operands without deriving namespaces, configuration files or generated outputs from filenames.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * requires supplied roots/final destination; [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * limits updates to explicitly supported settings and outputs.
 */
#include "consume.h"
#include "driver/options/validation/validation.h"

/** Scalar authority occurs once; repeated configuration/output entries retain argument order within argc-bounded arrays. Component contract: [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01), [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03). */
bool driver_options_relocation(DriverOptions *options, DriverOptionKind kind, const char *value) {
    if (*value == 0) { return driver_options_invalid(options,"Relocation option values must be nonempty"); }
    switch (kind) {
        case DRIVER_OPTION_DESTINATION:
            if (options->destination != NULL) { return driver_options_invalid(options,"Relocation accepts one --destination"); }
            options->destination = value;
            break;
        case DRIVER_OPTION_DESTINATION_ROOT: case DRIVER_OPTION_DESTINATION_STANDARD_ROOT:
            if (options->destination_root.path != NULL) { return driver_options_invalid(options,"Relocation accepts one explicit destination root kind/path"); }
            options->destination_root.path = value;
            options->destination_root.kind = kind == DRIVER_OPTION_DESTINATION_ROOT ? MODULE_ROOT_PROJECT : MODULE_ROOT_STANDARD;
            break;
        case DRIVER_OPTION_RECOVERY_DIRECTORY:
            if (options->recovery_directory != NULL) { return driver_options_invalid(options,"Relocation accepts one --recovery directory"); }
            options->recovery_directory = value;
            break;
        case DRIVER_OPTION_ENTRY:
            if (options->entry != NULL || !module_path_valid(value)) { return driver_options_invalid(options,"Relocation accepts one exact --entry module identity"); }
            options->entry = value;
            break;
        case DRIVER_OPTION_CONFIGURATION: options->configurations[options->configuration_count++] = value; break;
        case DRIVER_OPTION_INVALIDATION: options->invalidations[options->invalidation_count++] = value; break;
        default: return driver_options_invalid(options,"Unsupported relocation option destination");
    }
    return true;
}
