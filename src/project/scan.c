#include "project/internal.h"
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ModuleFile G_Files[MAX_MODULE_FILES];
int G_FileCount = 0;

static void join_path(char* out, size_t capacity, const char* prefix,
                       const char* name, const char* kind) {
    int length = prefix[0] ? snprintf(out, capacity, "%s/%s", prefix, name)
                           : snprintf(out, capacity, "%s", name);
    if (length < 0 || (size_t)length >= capacity) {
        fprintf(stderr, "error: %s too long: '%s/%s'\n", kind, prefix, name);
        exit(1);
    }
}

static void register_module_file(const char* root, const char* relative,
                                  const char* absolute) {
    if (G_FileCount >= MAX_MODULE_FILES) {
        fprintf(stderr, "error: too many module files under '%s' (max %d)\n",
                root, MAX_MODULE_FILES);
        exit(1);
    }
    ModuleFile* file = &G_Files[G_FileCount++];
    /* join_path checked these lengths against the same capacities. */
    memcpy(file->rel_path, relative, strlen(relative) + 1);
    memcpy(file->abs_path, absolute, strlen(absolute) + 1);
}

static void scan_entry(const char* root, const char* prefix,
                         const char* directory, const char* name) {
    char absolute[MAX_PATH_LINK_LEN];
    char relative[MAX_PATH_LEN];
    join_path(absolute, sizeof(absolute), directory, name, "path");
    join_path(relative, sizeof(relative), prefix, name, "relative path");
    struct stat info;
    if (stat(absolute, &info) != 0) {
        fprintf(stderr, "error: stat '%s': %s\n", absolute, strerror(errno));
        exit(1);
    }
    if (S_ISDIR(info.st_mode))
        scan_dir(root, relative);
    else if (S_ISREG(info.st_mode))
        register_module_file(root, relative, absolute);
}

/* As before, stat follows symlinks; callers must avoid directory symlink cycles. */
void scan_dir(const char* root, const char* prefix) {
    char path[MAX_PATH_LINK_LEN];
    if (prefix[0])
        join_path(path, sizeof(path), root, prefix, "path");
    else
        join_path(path, sizeof(path), "", root, "path");
    DIR* directory = opendir(path);
    if (!directory) {
        fprintf(stderr, "error: cannot open directory '%s': %s\n", path, strerror(errno));
        exit(1);
    }
    struct dirent* entry;
    while ((entry = readdir(directory)) != NULL) {
        if (entry->d_name[0] != '.')
            scan_entry(root, prefix, path, entry->d_name);
    }
    closedir(directory);
}

ModuleFile* find_module_file(const char* path) {
    for (int i = 0; i < G_FileCount; i++)
        if (strcmp(G_Files[i].rel_path, path) == 0)
            return &G_Files[i];
    return NULL;
}
