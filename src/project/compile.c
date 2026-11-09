#include "project/internal.h"
#include "common/path.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static bool load_project(
    Project* project,
    const char* project_root,
    const char* entry_module,
    const char* library_directory,
    Error* error
) {
    if (!scan_dir(project, project_root, "", error)) return false;
    if (!project->file_count)
        return error_set(error, ERROR_INVALID, 0, "no module files found under '%s'", project_root);
    return load_standard_library(project, library_directory, error) &&
            parse_project_declarations(project, error) && validate_entry(project, entry_module, error) &&
            validate_native_modules(project, true, error);
}

/* This is the adapter between symbols and the otherwise independent graph. */
static bool build_dependencies(Project* project, Error* error) {
    for (uint32_t index = 0; index < project->symbols.symbol_count; index++) {
        Symbol* symbol = &project->symbols.symbols[index];
        if (graph_add_node(&project->graph, symbol->module_path, error) < 0) return false;
        if (symbol->kind == SYMBOL_MODULE_ALIAS) {
            char callee_module_path[MAX_MODULE_PATH_LENGTH];
            if (!normalize_path(symbol->alias_path, callee_module_path, error) ||
                !graph_add_dependency(&project->graph, symbol->module_path, callee_module_path, error))
                return false;
        }
    }
    return true;
}

static bool emit_modules(Project* project, const int* dependency_order, Error* error) {
    for (int index = 0; index < project->graph.node_count; index++) {
        const char* module_path = graph_path(&project->graph, dependency_order[index]);
        ModuleFile* module = find_module_file(project, module_path);
        if (!module)
            return error_set(error, ERROR_INVALID, 0, "module '%s' has no source file", module_path);
        if (!emit_project_module(project, module, error)) return false;
    }
    return true;
}

static bool write_output(Project* project, const char* output_path, Error* error) {
    FILE* function_bodies = project->emitter.output;
    if (fflush(function_bodies) || fseek(function_bodies, 0, SEEK_SET))
        return error_set(error, ERROR_IO, 0, "cannot rewind generated output");
    FILE* output = fopen(output_path, "wb");
    if (!output) return error_set(error, ERROR_IO, 0, "cannot open '%s': %s", output_path, strerror(errno));
    emit_program_prologue(output, &project->symbols, &project->graph);
    char buffer[4096];
    size_t length;
    bool success = true;
    while ((length = fread(buffer, 1, sizeof(buffer), function_bodies)) > 0) {
        if (fwrite(buffer, 1, length, output) != length) { success = false; break; }
    }
    if (ferror(function_bodies)) success = false;
    if (fclose(output)) success = false;
    if (!success) return error_set(error, ERROR_IO, 0, "cannot write '%s'", output_path);
    return true;
}

static bool generate_program(
    Project* project,
    const char* entry_module,
    const char* output_path,
    Error* error
) {
    int dependency_order[MAX_MODULES];
    if (!build_dependencies(project, error) || !graph_sort(&project->graph, dependency_order, error))
        return false;
    if (!parse_project_bodies(project, dependency_order, error) ||
        !validate_project_bodies(project, dependency_order, error) ||
        !validate_generated_names(project, error)) return false;
    project->emitter.source_mapping_enabled = true;
    project->emitter.output = tmpfile();
    if (!project->emitter.output)
        return error_set(error, ERROR_IO, 0, "cannot create temporary output");
    if (!emit_modules(project, dependency_order, error)) return false;
    emit_program_entry_point(&project->emitter, entry_module);
    char directory[MAX_FILE_PATH_LENGTH];
    return output_directory(output_path, directory, sizeof(directory), error) &&
        emit_native_headers(project, directory, error) && write_output(project, output_path, error);
}

bool compile_project_with_library(
    const char* project_root,
    const char* entry_module,
    const char* output_path,
    const char* library_directory,
    Error* error
) {
    error_clear(error);
    Project* project = calloc(1, sizeof(*project));
    if (!project) return error_set(error, ERROR_MEMORY, 0, "out of memory (project)");
    symbols_init(&project->symbols);
    graph_init(&project->graph);
    emitter_init(&project->emitter, NULL, &project->symbols);
    bool success = load_project(project, project_root, entry_module, library_directory, error) &&
                    generate_program(project, entry_module, output_path, error);
    if (project->emitter.output) {
        int status = fclose(project->emitter.output);
        if (status && success)
            success = error_set(error, ERROR_IO, 0, "cannot close generated output");
    }
    destroy_project_syntax(project);
    free(project);
    return success;
}

bool compile_project(
    const char* project_root,
    const char* entry_module,
    const char* output_path,
    Error* error
) {
    return compile_project_with_library(project_root, entry_module, output_path,
        selected_standard_library_directory(), error);
}

static bool export_c_interfaces(
    const char* project_root,
    const char* header_directory,
    bool create_sources,
    Error* error
) {
    error_clear(error);
    Project* project = calloc(1, sizeof(*project));
    if (!project) return error_set(error, ERROR_MEMORY, 0, "out of memory (project)");
    symbols_init(&project->symbols);
    graph_init(&project->graph);
    bool success = scan_dir(project, project_root, "", error) &&
        load_standard_library(project, selected_standard_library_directory(), error) &&
        parse_project_declarations(project, error) &&
        validate_native_modules(project, !create_sources, error) &&
        prepare_header_directory(header_directory, error) &&
        emit_native_headers(project, header_directory, error);
    if (success && create_sources) success = create_native_stubs(project, error);
    destroy_project_syntax(project);
    free(project);
    return success;
}

bool generate_c_stubs(const char* project_root, const char* header_directory, Error* error) {
    return export_c_interfaces(project_root, header_directory, true, error);
}

bool generate_c_headers(const char* project_root, const char* header_directory, Error* error) {
    return export_c_interfaces(project_root, header_directory, false, error);
}
