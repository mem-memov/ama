/** @file
 * @brief Admit only a unique same-version canonical proposed main namespace.
 * [PKG-01](../../../../../spec/project/packages.md#pkg-01) determines ordinary
 * version boundaries; [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * disallows merging distinct final nominal names or inventing missing endpoints.
 */
#include "internal.h"
#include "source/path/logical.h"
#include <string.h>
/** A module must have the exact requested canonical version boundary. */
static bool in_version(const char *identity, const char *version, size_t length) {
    ModulePathShape shape;
    return identity && module_path_describe((const unsigned char *)identity,
        strlen(identity), &shape) && shape.extent == MODULE_PATH_MODULE &&
        shape.root == MODULE_ROOT_PROJECT && shape.version_length == length &&
        !memcmp(identity, version, length);
}
bool source_main_placement_valid(const SourceVersionPlacement *version,
    const char *const *mains, size_t count, const char *identity, size_t *length) {
    ModulePathShape shape;
    if (!version || !version->identity || !version->physical || version->physical[0] != '/' ||
        (count && !mains) || !length ||
        !module_path_describe((const unsigned char *)version->identity, strlen(version->identity), &shape) ||
        shape.root != MODULE_ROOT_PROJECT || shape.extent != MODULE_PATH_VERSION) { return false; }
    size_t version_length = strlen(version->identity);
    if (!in_version(identity, version->identity, version_length)) { return false; }
    for (size_t i = 0; i < count; i++) {
        if (!in_version(mains[i], version->identity, version_length)) { return false; }
        for (size_t j = 0; j < i; j++) { if (!strcmp(mains[i], mains[j])) { return false; } }
    }
    *length = version_length;
    return true;
}
