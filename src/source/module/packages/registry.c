/**
 * @file
 * @brief Publish one selected physical version per exact logical package identity.
 *
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) selects local versions
 * before external candidates, rejects competing same-priority copies and forbids
 * filling local holes from any ignored external tree.
 */
#include <string.h>
#include "internal.h"

/** Local-first discovery makes ignoring external candidates a pure identity query.
 * [PKG-02](../../../../spec/project/packages.md#pkg-02) requires this decision
 * before inspecting the ignored version's modules/configuration/companions.
 */
bool package_ignored(const PackageScan *scan, const char *logical) {
    if (scan->local) {
        return false;
    }
    for (const ModulePackage *package = *scan->selected; package != NULL; package = package->next) {
        int order = strcmp(package->logical, logical);
        if (order > 0) {
            break;
        }
        if (order == 0) {
            return package->local;
        }
    }
    return false;
}

/** Insert at the unique sorted position; fields are published only after allocation.
 * A repeated name is an error rather than directory-order shadowing under
 * [PKG-02](../../../../spec/project/packages.md#pkg-02). The caller already
 * discarded lower-priority candidates, so no replacement/rollback is needed.
 */
ModuleStatus package_register(PackageScan *scan, const char *physical,
    const char *relative, const char *logical) {
    ModulePackage **position = scan->selected;
    while (*position != NULL && strcmp((*position)->logical, logical) < 0) {
        position = &(*position)->next;
    }
    if (*position != NULL && strcmp((*position)->logical, logical) == 0) {
        scan->catalog->issue.related_path = (*position)->physical;
        return module_fail(scan->catalog, MODULE_AMBIGUOUS, physical);
    }
    ModulePackage *package = source_arena_allocate(&scan->catalog->arena, sizeof(*package));
    if (package == NULL) {
        return module_fail(scan->catalog, MODULE_OUT_OF_MEMORY, physical);
    }
    *package = (ModulePackage){*position, scan->root, physical, relative, logical, scan->local};
    *position = package;
    return MODULE_OK;
}
