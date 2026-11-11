#include "../internal.h"

ModuleStatus module_remember_directory(ModuleCatalog *catalog, const char *path) {
    struct stat metadata;
    if (lstat(path, &metadata) != 0) {
        return module_fail(catalog, MODULE_IO_ERROR, path);
    }
    if (!S_ISDIR(metadata.st_mode)) {
        return module_fail(catalog, MODULE_UNSUPPORTED_PATH, path);
    }
    for (ModuleDirectoryIdentity *other = catalog->directories; other != NULL; other = other->next) {
        if (other->device == metadata.st_dev && other->inode == metadata.st_ino) {
            catalog->issue.related_path = other->path;
            return module_fail(catalog, MODULE_FILE_ALIAS, path);
        }
    }
    ModuleDirectoryIdentity *identity = source_arena_allocate(&catalog->arena, sizeof(*identity));
    if (identity == NULL) {
        return module_fail(catalog, MODULE_OUT_OF_MEMORY, path);
    }
    *identity = (ModuleDirectoryIdentity){catalog->directories, path, metadata.st_dev, metadata.st_ino};
    catalog->directories = identity;
    return MODULE_OK;
}
