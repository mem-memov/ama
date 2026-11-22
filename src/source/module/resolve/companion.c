/** @file
 * @brief Observe one exact optional companion without publishing a partial activation.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) locates companions
 * beside the physical source. [PKG-03](../../../../spec/project/packages.md#pkg-03)
 * requires UTF-8 configuration text. [DEP-03](../../../../spec/implementation/compiler.md#dep-03)
 * distinguishes absence from a contributing empty file.
 */
#include <string.h>
#include "../internal.h"

/** Own path/scratch before probing; publish absent or complete regular-file observations.
 * [SRC-01](../../../../spec/language/source-and-names.md#src-01) forbids alternate
 * spellings and symlink fallback; [PKG-03](../../../../spec/project/packages.md#pkg-03)
 * validates original text through the shared UTF-8 loader. No input is mutated.
 */
ModuleStatus module_read_companion(ModuleCatalog *catalog, const char *physical,
    const char *suffix, bool text, ModuleCompanion *output) {
    const char *path = source_arena_join(&catalog->arena, physical, "", suffix);
    char *parent = source_arena_copy(&catalog->arena, physical);
    if (path == NULL || parent == NULL) {
        return module_fail(catalog, MODULE_OUT_OF_MEMORY, physical);
    }
    char *slash = strrchr(parent, '/');
    const char *name = strrchr(path, '/');
    if (slash != NULL) { *slash = 0; }
    struct stat metadata;
    ModuleIssue previous = catalog->issue;
    ModuleStatus probed = module_probe(catalog, slash == NULL ? "." : (*parent == 0 ? "/" : parent),
        name == NULL ? path : name + 1, path, &metadata);
    if (probed == MODULE_MISSING) {
        /* Optional absence is a successful observation, not a retained failure
         * from the shared physical/projected exact probe. */
        catalog->issue = previous;
        *output = (ModuleCompanion){.path = path};
        return MODULE_OK;
    }
    if (probed != MODULE_OK) { return probed; }
    if (!S_ISREG(metadata.st_mode)) { return module_fail(catalog, MODULE_UNSUPPORTED_PATH, path); }
    ModuleCompanion prepared = {.path = path, .present = true};
    ModuleStatus status = text ? module_load_utf8(catalog, path, &prepared.text) : MODULE_OK;
    if (status == MODULE_OK) { *output = prepared; }
    return status;
}
