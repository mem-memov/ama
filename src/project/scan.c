#include "project/internal.h"
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

static bool join_path(char* out, size_t capacity, const char* prefix,
                       const char* name, Error* error) {
    int length = prefix[0] ? snprintf(out, capacity, "%s/%s", prefix, name)
                           : snprintf(out, capacity, "%s", name);
    if (length < 0 || (size_t)length >= capacity)
        return error_set(error, ERROR_LIMIT, 0, "path too long: '%s/%s'", prefix, name);
    return true;
}

static bool register_module_file(Project* project, const char* relative,
                                  const char* absolute, Error* error) {
    if (project->file_count == MAX_MODULE_FILES)
        return error_set(error, ERROR_LIMIT, 0, "too many module files (max %d)", MAX_MODULE_FILES);
    ModuleFile* file = &project->files[project->file_count++];
    strcpy(file->rel_path, relative);
    strcpy(file->abs_path, absolute);
    return graph_add_node(&project->graph, relative, error) >= 0;
}

static bool scan_entry(Project* project, const char* root, const char* prefix,
                         const char* directory, const char* name, Error* error) {
    char absolute[MAX_PATH_LINK_LEN];
    char relative[MAX_PATH_LEN];
    if (!join_path(absolute, sizeof(absolute), directory, name, error) ||
        !join_path(relative, sizeof(relative), prefix, name, error)) return false;
    struct stat info;
    if (stat(absolute, &info) != 0)
        return error_set(error, ERROR_IO, 0, "stat '%s': %s", absolute, strerror(errno));
    if (S_ISDIR(info.st_mode)) return scan_dir(project, root, relative, error);
    if (S_ISREG(info.st_mode)) return register_module_file(project, relative, absolute, error);
    return true;
}

/* stat follows symlinks; source trees must avoid directory symlink cycles. */
bool scan_dir(Project* project, const char* root, const char* prefix, Error* error) {
    char path[MAX_PATH_LINK_LEN];
    if (!join_path(path, sizeof(path), prefix[0] ? root : "", prefix[0] ? prefix : root, error))
        return false;
    DIR* directory = opendir(path);
    if (!directory)
        return error_set(error, ERROR_IO, 0, "cannot open directory '%s': %s", path, strerror(errno));
    bool success = true;
    struct dirent* entry;
    while (success) {
        errno = 0;
        entry = readdir(directory);
        if (!entry) {
            if (errno) success = error_set(error, ERROR_IO, 0, "cannot read directory '%s'", path);
            break;
        }
        if (entry->d_name[0] != '.')
            success = scan_entry(project, root, prefix, path, entry->d_name, error);
    }
    closedir(directory);
    return success;
}

ModuleFile* find_module_file(Project* project, const char* path) {
    for (int i = 0; i < project->file_count; i++)
        if (!strcmp(project->files[i].rel_path, path)) return &project->files[i];
    return NULL;
}
