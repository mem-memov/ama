/** @file
 * @brief Validate lexical recovery paths before any journal-controlled filesystem operation.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * prevents ambiguous spelling from changing the set of authorized recovery locations.
 */
#include <string.h>
#include "../internal.h"

/** Every slash consumes exactly one nonempty component; root alone is canonical. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_recovery_path(const char *path) {
    if (path == NULL || path[0] != '/') { return false; }
    if (path[1] == 0) { return true; }
    const char *component = path+1;
    for (const char *cursor = component;; cursor++) {
        if (*cursor != '/' && *cursor != 0) { continue; }
        size_t length = (size_t)(cursor-component);
        if (length == 0 || (length == 1 && component[0] == '.')
            || (length == 2 && component[0] == '.' && component[1] == '.')) { return false; }
        if (*cursor == 0) { return true; }
        component = cursor+1;
    }
}
