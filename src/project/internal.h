#ifndef AMA_PROJECT_INTERNAL_H
#define AMA_PROJECT_INTERNAL_H

#include "project/project.h"
#include "common/limits.h"

typedef struct {
    char rel_path[MAX_PATH_LEN];
    char abs_path[MAX_PATH_LINK_LEN];
} ModuleFile;
extern ModuleFile G_Files[MAX_MODULE_FILES];
extern int G_FileCount;
void scan_dir(const char* root, const char* prefix);
ModuleFile* find_module_file(const char* path);
void pass1_declarations(void);
void pass2_module_code(const char* rel_path, const char* abs_path);
void validate_entry(const char* entry);

#endif
