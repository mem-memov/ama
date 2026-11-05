#ifndef AMA_PROJECT_INTERNAL_H
#define AMA_PROJECT_INTERNAL_H
#include "project/project.h"
#include "symbols/table.h"
#include "graph/graph.h"
#include "codegen/emitter.h"

typedef struct {
    char rel_path[MAX_PATH_LEN];
    char abs_path[MAX_PATH_LINK_LEN];
} ModuleFile;

typedef struct {
    SymbolTable symbols;
    Graph graph;
    ModuleFile files[MAX_MODULE_FILES];
    int file_count;
    Emitter emitter;
} Project;

bool scan_dir(Project* project, const char* root, const char* prefix, Error* error);
ModuleFile* find_module_file(Project* project, const char* path);
bool pass1_declarations(Project* project, Error* error);
bool pass2_module_code(Project* project, const ModuleFile* module, Error* error);
bool validate_entry(Project* project, const char* entry, Error* error);
#endif
