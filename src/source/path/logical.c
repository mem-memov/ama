/**
 * @file
 * @brief Check canonical namespace and package structure with a bounded forward scan.
 *
 * [SRC-01](../../../spec/language/source-and-names.md#src-01) requires complete
 * logical endpoints. [PKG-01](../../../spec/project/packages.md#pkg-01) makes the
 * first complete version component the boundary after vendor/package names;
 * [SRC-03](../../../spec/language/source-and-names.md#src-03) supplies exact names.
 */
#include <string.h>
#include "logical.h"
#include "version.h"
#include "source/unicode/unicode.h"

/** Prefix states prevent a version or source area from acting as a module name.
 * [PKG-01](../../../spec/project/packages.md#pkg-01) requires one vendor, at
 * least one package component, one version and a src/tests area before modules.
 */
typedef enum {
    PATH_VENDOR,
    PATH_PACKAGES,
    PATH_AREA,
    PATH_MODULES
} PathPhase;

/** Compare the already delimited area against its two exact ASCII spellings.
 * [PKG-01](../../../spec/project/packages.md#pkg-01) retains src/tests in logical
 * identity; they are neither aliases nor inferred from physical root names.
 */
static bool source_area(const unsigned char *bytes, size_t length) {
    return (length == 3 && memcmp(bytes, "src", 3) == 0)
        || (length == 5 && memcmp(bytes, "tests", 5) == 0);
}

/** Scan disjoint nonempty components. Every separator advances the byte cursor;
 * each component takes a constant number of linear word/version validations.
 * The phase records the exact consumed prefix, and counts are bounded by the
 * input's positive component widths, so neither iteration nor arithmetic needs
 * a guessed depth/size cap. [SRC-01](../../../spec/language/source-and-names.md#src-01)
 * separates root kinds; [PKG-01](../../../spec/project/packages.md#pkg-01) fixes
 * the ordinary endpoint shape without looking at files or parsing integer values.
 */
bool module_path_canonical(const unsigned char *bytes, size_t length) {
    if (bytes == NULL || length == 0 || bytes[0] != '/') {
        return false;
    }
    bool standard = length > 1 && bytes[1] == '/';
    size_t cursor = standard ? 2 : 1;
    PathPhase phase = PATH_VENDOR;
    size_t package_count = 0;
    size_t module_count = 0;
    if (cursor == length) {
        return false;
    }
    for (;;) {
        size_t end = cursor;
        while (end < length && bytes[end] != '/') {
            end++;
        }
        size_t width = end - cursor;
        size_t error;
        bool name = unicode_name(bytes + cursor, width, &error);
        if (standard) {
            if (!name) {
                return false;
            }
            module_count++;
        } else if (phase == PATH_VENDOR) {
            if (!name) {
                return false;
            }
            phase = PATH_PACKAGES;
        } else if (phase == PATH_PACKAGES) {
            if (module_version_component(bytes + cursor, width)) {
                if (package_count == 0) {
                    return false;
                }
                phase = PATH_AREA;
            } else {
                if (!name) {
                    return false;
                }
                package_count++;
            }
        } else if (phase == PATH_AREA) {
            if (!source_area(bytes + cursor, width)) {
                return false;
            }
            phase = PATH_MODULES;
        } else {
            if (!name) {
                return false;
            }
            module_count++;
        }
        if (end == length) {
            return module_count != 0;
        }
        cursor = end + 1;
    }
}
