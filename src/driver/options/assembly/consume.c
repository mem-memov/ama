/**
 * @file
 * @brief Consume each exact CLI option once, with no inferred flag/value syntax.
 *
 * [COMP-05](../../../../spec/implementation/compiler.md#comp-05) requires a
 * deterministic request. Scalar destinations retain the existing last-value
 * behavior; root/exclusion/C-flag entries retain argv order, and flags are idempotent.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) keeps source/--root
 * entries in the project/import family; --standard-root explicitly selects the
 * standard family, never guessing authority from paths.
 */
#include "consume.h"
#include "driver/options/recognition/option.h"
#include "driver/options/validation/validation.h"

/** Install one recognized value or flag; caller has already checked its arity.
 * Each append consumes a distinct argv entry, bounded by the allocated count.
 * Component contract: [COMP-05](../../../../spec/implementation/compiler.md#comp-05), [SRC-01](../../../../spec/language/source-and-names.md#src-01). */
static bool install(DriverOptions *options, DriverOptionStorage *storage,
    DriverOptionKind kind, const char *value) {
    switch (kind) {
        case DRIVER_OPTION_ROOT: case DRIVER_OPTION_STANDARD_ROOT:
            options->roots[options->root_count++] = (ModuleRoot){.path = value,
                .kind = kind == DRIVER_OPTION_STANDARD_ROOT ? MODULE_ROOT_STANDARD : MODULE_ROOT_PROJECT};
            break;
        case DRIVER_OPTION_EXCLUDE: storage->exclusions[storage->excluded++] = value; break;
        case DRIVER_OPTION_OUTPUT: options->output = value; break;
        case DRIVER_OPTION_SDK: options->sdk = value; break;
        case DRIVER_OPTION_COMPILER: options->compiler = value; break;
        case DRIVER_OPTION_PROTOCOLS: options->protocols = value; break;
        case DRIVER_OPTION_C_FLAG: options->c_flags[options->c_flag_count++] = value; break;
        case DRIVER_OPTION_COMPILE: options->compile = true; break;
        case DRIVER_OPTION_COMPONENT: case DRIVER_OPTION_DELIVERY_MODE:
            return driver_options_delivery(options, kind, value);
        case DRIVER_OPTION_CREATE: options->create = true; break;
        case DRIVER_OPTION_OVERWRITE: options->overwrite = true; break;
        case DRIVER_OPTION_QUERY_MODULE: case DRIVER_OPTION_OFFSET: case DRIVER_OPTION_RENAME_NAME:
            return driver_options_source_tool(options,kind,value);
        case DRIVER_OPTION_DESTINATION: case DRIVER_OPTION_DESTINATION_ROOT: case DRIVER_OPTION_DESTINATION_STANDARD_ROOT:
        case DRIVER_OPTION_RECOVERY_DIRECTORY: case DRIVER_OPTION_ENTRY: case DRIVER_OPTION_CONFIGURATION: case DRIVER_OPTION_INVALIDATION:
            return driver_options_relocation(options,kind,value);
    }
    return true;
}

bool driver_options_consume(DriverOptions *options, DriverOptionStorage *storage,
    int count, char *const arguments[]) {
    for (int index = 4; index < count;) {
        const DriverOption *option = driver_option_recognize(options->request, arguments[index++]);
        if (option == NULL) {
            return driver_options_invalid(options, index == count ? "Command option needs a value" : "Unknown command option");
        }
        if (option->takes_value && index == count) {
            return driver_options_invalid(options, "Command option needs a value");
        }
        const char *value = option->takes_value ? arguments[index++] : NULL;
        if (!install(options, storage, option->kind, value)) { return false; }
    }
    return true;
}
