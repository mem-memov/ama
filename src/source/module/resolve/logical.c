/**
 * @file
 * @brief Adapt retained C-string lookup keys to the shared canonical identity predicate.
 *
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) distinguishes
 * ordinary/standard slash namespace selectors;
 * [PKG-01](../../../../spec/project/packages.md#pkg-01) supplies the ordinary
 * package-version shape. Share bounded recognition with parser/source consumers.
 */
#include "../catalog.h"
#include "source/path/logical.h"
#include <string.h>

/** Validate an existing terminated lookup key without copying or normalizing it.
 * The C-string boundary differs from bounded source spans; both share the same
 * canonical kernel under [SRC-01](../../../../spec/language/source-and-names.md#src-01)
 * and [PKG-01](../../../../spec/project/packages.md#pkg-01).
 */
bool module_path_valid(const char *path) {
    return path != NULL && module_path_canonical((const unsigned char *)path, strlen(path));
}
