#ifndef AMA_PROJECT_INTERNAL_H
#define AMA_PROJECT_INTERNAL_H
#include "project/project.h"
#include "symbols/table.h"
#include "graph/graph.h"
#include "codegen/emitter.h"

typedef struct {
    char module_path[MAX_MODULE_PATH_LENGTH];
    char file_path[MAX_FILE_PATH_LENGTH];
} ModuleFile;

typedef struct {
    SymbolTable symbols;
    Graph graph;
    ModuleFile files[MAX_MODULE_FILES];
    int file_count;
    Emitter emitter;
} Project;

bool scan_dir(Project* project, const char* project_root, const char* relative_prefix, Error* error);
ModuleFile* find_module_file(Project* project, const char* path);
bool parse_project_declarations(Project* project, Error* error);
bool emit_project_module(Project* project, const ModuleFile* module, Error* error);
bool validate_entry(Project* project, const char* entry_module, Error* error);
#endif
