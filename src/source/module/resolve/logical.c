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

