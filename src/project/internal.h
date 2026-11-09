#ifndef AMA_PROJECT_INTERNAL_H
#define AMA_PROJECT_INTERNAL_H
#include "project/project.h"
#include "symbols/table.h"
#include "graph/graph.h"
#include "codegen/emitter.h"
#include "lexer/sections.h"
#include "ast/ast.h"

typedef struct {
    char module_path[MAX_MODULE_PATH_LENGTH];
    char file_path[MAX_FILE_PATH_LENGTH];
    char source_path[MAX_FILE_PATH_LENGTH];
    ModuleImplementation implementation;
    bool is_standard_library;
    AstModule syntax;
} ModuleFile;

typedef struct {
    SymbolTable symbols;
    Graph graph;
    ModuleFile files[MAX_MODULE_FILES];
    int file_count;
    Emitter emitter;
} Project;

const char* selected_standard_library_directory(void);
bool load_standard_library(Project* project, const char* directory, Error* error);
bool scan_dir(Project* project, const char* project_root, const char* relative_prefix, Error* error);
ModuleFile* find_module_file(Project* project, const char* path);
bool parse_project_declarations(Project* project, Error* error);
bool parse_project_bodies(Project* project, const int* dependency_order, Error* error);
bool validate_project_bodies(Project* project, const int* dependency_order, Error* error);
void destroy_project_syntax(Project* project);
bool emit_project_module(Project* project, const ModuleFile* module, Error* error);
bool validate_generated_names(Project* project, Error* error);
bool validate_native_modules(Project* project, bool require_sources, Error* error);
bool emit_native_headers(Project* project, const char* directory, Error* error);
bool create_native_stubs(Project* project, Error* error);
bool prepare_header_directory(const char* directory, Error* error);
bool output_directory(const char* path, char* directory, size_t capacity, Error* error);
bool validate_entry(Project* project, const char* entry_module, Error* error);
#endif
