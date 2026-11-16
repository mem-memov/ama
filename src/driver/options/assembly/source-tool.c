/** @file
 * @brief Parse exact source-tool selectors without offsets inferred from display columns or lossy numeric conversions.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * identifies source positions by original UTF-8 byte offsets, including EOF.
 */
#include <stdint.h>
#include "consume.h"
#include "driver/options/recognition/option.h"
#include "driver/options/validation/validation.h"

/** Canonical unsigned decimal consumes every argument byte and checks multiply/add before publishing. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
static bool offset(const char *text, size_t *output) {
    if (*text < '0' || *text > '9' || (text[0] == '0' && text[1] != 0)) { return false; }
    size_t value = 0;
    for (const unsigned char *cursor = (const unsigned char *)text; *cursor != 0; cursor++) {
        if (*cursor < '0' || *cursor > '9' || value > (SIZE_MAX-(*cursor-'0'))/10) { return false; }
        value = value*10+(*cursor-'0');
    }
    *output = value;
    return true;
}
/** Reject duplicates rather than silently choosing one source selector; arguments remain borrowed. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool driver_options_source_tool(DriverOptions *options, DriverOptionKind kind, const char *value) {
    if (kind == DRIVER_OPTION_QUERY_MODULE) {
        if (options->query_module != NULL || !module_path_valid(value)) {
            return driver_options_invalid(options,"Expected one exact --module logical path");
        }
        options->query_module = value;
    } else if (kind == DRIVER_OPTION_RENAME_NAME) {
        if (options->rename_name != NULL || *value == 0) { return driver_options_invalid(options,"Expected one nonempty --name"); }
        options->rename_name = value;
    } else if (kind == DRIVER_OPTION_OFFSET) {
        if (options->has_offset || !offset(value,&options->offset)) {
            return driver_options_invalid(options,"Expected one canonical nonnegative --offset in the supported byte range");
        }
        options->has_offset = true;
    } else { return driver_options_invalid(options,"Unsupported source-tool option destination"); }
    return true;
}
