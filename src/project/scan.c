#include "project/internal.h"
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

static bool join_path(char* combined_path, size_t capacity, const char* relative_prefix,
    const char* name, Error* error) {
    int length = relative_prefix[0] ? snprintf(combined_path, capacity, "%s/%s", relative_prefix, name)
                            : snprintf(combined_path, capacity, "%s", name);
    if (length < 0 || (size_t)length >= capacity)
        return error_set(error, ERROR_LIMIT, 0, "path too long: '%s/%s'", relative_prefix, name);
    return true;
}

static bool register_module_file(Project* project, const char* module_path,
    const char* file_path, Error* error) {
    if (project->file_count == MAX_MODULE_FILES)
        return error_set(error, ERROR_LIMIT, 0, "too many module files (max %d)", MAX_MODULE_FILES);
    ModuleFile* file = &project->files[project->file_count++];
    strcpy(file->module_path, module_path);
    strcpy(file->file_path, file_path);
    file->implementation = MODULE_DECLARATION_ONLY;
    return true;
}

static bool scan_entry(Project* project, const char* project_root, const char* relative_prefix,
    const char* directory, const char* name, Error* error) {
    char file_path[MAX_FILE_PATH_LENGTH];
    char module_path[MAX_MODULE_PATH_LENGTH];
    if (!join_path(file_path, sizeof(file_path), directory, name, error) ||
        !join_path(module_path, sizeof(module_path), relative_prefix, name, error)) return false;
    struct stat file_information;
    if (stat(file_path, &file_information) != 0)
        return error_set(error, ERROR_IO, 0, "stat '%s': %s", file_path, strerror(errno));
    if (S_ISDIR(file_information.st_mode)) return scan_dir(project, project_root, module_path, error);
    const char* extension = strrchr(name, '.');
    if (extension && (!strcmp(extension, ".c") || !strcmp(extension, ".h"))) return true;
    if (S_ISREG(file_information.st_mode)) return register_module_file(project, module_path, file_path, error);
    return true;
}

/* stat follows symlinks; source trees must avoid directory symlink cycles. */
bool scan_dir(Project* project, const char* project_root, const char* relative_prefix, Error* error) {
    char path[MAX_FILE_PATH_LENGTH];
    if (!join_path(path, sizeof(path), relative_prefix[0] ? project_root : "", relative_prefix[0] ? relative_prefix : project_root, error))
        return false;
    DIR* directory = opendir(path);
    if (!directory)
        return error_set(error, ERROR_IO, 0, "cannot open directory '%s': %s", path, strerror(errno));
    bool success = true;
    struct dirent* directory_entry;
    while (success) {
        errno = 0;
        directory_entry = readdir(directory);
        if (!directory_entry) {
            if (errno) success = error_set(error, ERROR_IO, 0, "cannot read directory '%s'", path);
            break;
        }
        if (directory_entry->d_name[0] != '.')
            success = scan_entry(project, project_root, relative_prefix, path, directory_entry->d_name, error);
    }
    closedir(directory);
    return success;
}

ModuleFile* find_module_file(Project* project, const char* path) {
    for (int index = 0; index < project->file_count; index++)
        if (!strcmp(project->files[index].module_path, path)) return &project->files[index];
    return NULL;
}
