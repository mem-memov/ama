/** @file
 * @brief Consume exact explicit output-mode and component authority without guessed defaults.
 * [CLI-01](../../../../spec/implementation/launch-and-embedding.md#cli-01)
 * separates compile-time identity/mode from runtime inputs;
 * [EMBED-09](../../../../spec/implementation/c-library.md#embed-09)
 * preserves the complete selected component identity for public entry naming.
 */
#include <string.h>
#include "consume.h"
#include "driver/options/validation/validation.h"
#include "source/unicode/unicode.h"
bool driver_options_delivery(DriverOptions *options, DriverOptionKind kind, const char *value) {
    if (kind == DRIVER_OPTION_COMPONENT) {
        size_t offset;
        if (options->component || !value || !*value || !utf8_validate((const unsigned char *)value, strlen(value), &offset)) {
            return driver_options_invalid(options, "Delivery requires one nonempty UTF-8 --component value without repetition");
        }
        options->component = value;
        return true;
    }
    if (kind != DRIVER_OPTION_DELIVERY_MODE || options->has_delivery_mode || !value) {
        return driver_options_invalid(options, "Delivery requires exactly one --mode value");
    }
    if (!strcmp(value, "executable")) { options->delivery_mode = DELIVERY_EXECUTABLE; }
    else if (!strcmp(value, "source-inclusion")) { options->delivery_mode = DELIVERY_SOURCE_INCLUSION; }
    else { return driver_options_invalid(options, "Unknown delivery mode; expected executable or source-inclusion"); }
    options->has_delivery_mode = true;
    return true;
}
