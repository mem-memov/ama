/**
 * @file
 * @brief Validate the retained single-dot sequence of Unicode word components.
 *
 * [SRC-03](../../../../spec/language/source-and-names.md#src-03)
 * This is an existing restricted validator, not full multiword/two-root support; T014/T015 replace it.
 */
#include <string.h>
#include "../catalog.h"
#include "source/unicode/unicode.h"

bool module_path_valid(const char *path) {
    if (path == NULL || path[0] != '.' || path[1] == 0) {
        return false;
    }
    const char *start = path + 1;
    do {
        const char *end = strchr(start, '.');
        size_t length = end == NULL ? strlen(start) : (size_t)(end - start);
        size_t error;
        if (!unicode_identifier((const unsigned char *)start, length, &error)) {
            return false;
        }
        if (end == NULL) {
            return true;
        }
        start = end + 1;
    } while (true);
}

