#include "project/internal.h"
#include "common/path.h"
#include "codegen/intrinsics.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

static bool load_project(Project* project, const char* root, const char* entry, Error* error) {
    if (!scan_dir(project, root, "", error)) return false;
    if (!project->file_count)
        return error_set(error, ERROR_INVALID, 0, "no module files found under '%s'", root);
    return register_intrinsics(&project->symbols, error) &&
           pass1_declarations(project, error) && validate_entry(project, entry, error);
}

/* This is the adapter between symbols and the otherwise independent graph. */
static bool build_dependencies(Project* project, Error* error) {
    for (uint32_t i = 0; i < project->symbols.count; i++) {
        Symbol* symbol = &project->symbols.symbols[i];
        if (graph_add_node(&project->graph, symbol->module_path, error) < 0) return false;
        if (symbol->kind == SYM_ALIAS) {
            char path[MAX_PATH_LEN];
            if (!normalize_path(symbol->path_link, path, error) ||
                !graph_add_dependency(&project->graph, symbol->module_path, path, error))
                return false;
        }
    }
    return true;
}

static bool emit_modules(Project* project, const int* order, Error* error) {
    for (int i = 0; i < project->graph.count; i++) {
        const char* path = graph_path(&project->graph, order[i]);
        if (is_intrinsic_path(path)) continue;
        ModuleFile* module = find_module_file(project, path);
        if (!module)
            return error_set(error, ERROR_INVALID, 0, "module '%s' has no source file", path);
        if (!pass2_module_code(project, module, error)) return false;
    }
    return true;
}

static bool write_output(Project* project, const char* path, Error* error) {
    FILE* body = project->emitter.out;
    if (fflush(body) || fseek(body, 0, SEEK_SET))
        return error_set(error, ERROR_IO, 0, "cannot rewind generated output");
    FILE* out = fopen(path, "wb");
    if (!out) return error_set(error, ERROR_IO, 0, "cannot open '%s': %s", path, strerror(errno));
    backend_emit_prologue(out, &project->symbols);
    char buffer[4096];
    size_t length;
    bool success = true;
    while ((length = fread(buffer, 1, sizeof(buffer), body)) > 0) {
        if (fwrite(buffer, 1, length, out) != length) { success = false; break; }
    }
    if (ferror(body)) success = false;
    if (fclose(out)) success = false;
    if (!success) return error_set(error, ERROR_IO, 0, "cannot write '%s'", path);
    return true;
}

static bool generate_program(Project* project, const char* entry, const char* path, Error* error) {
    int order[MAX_MODULES];
    if (!build_dependencies(project, error) || !graph_sort(&project->graph, order, error))
        return false;
    project->emitter.out = tmpfile();
    if (!project->emitter.out)
        return error_set(error, ERROR_IO, 0, "cannot create temporary output");
    if (!backend_emit_intrinsics(project->emitter.out, &project->symbols, error) ||
        !emit_modules(project, order, error)) return false;
    backend_emit_main(&project->emitter, entry);
    return write_output(project, path, error);
}

bool compile_project(const char* root, const char* entry, const char* path, Error* error) {
    error_clear(error);
    Project* project = calloc(1, sizeof(*project));
    if (!project) return error_set(error, ERROR_MEMORY, 0, "out of memory (project)");
    symbols_init(&project->symbols);
    graph_init(&project->graph);
    emitter_init(&project->emitter, NULL, &project->symbols);
    bool success = load_project(project, root, entry, error) &&
                   generate_program(project, entry, path, error);
    if (project->emitter.out) {
        int status = fclose(project->emitter.out);
        if (status && success)
            success = error_set(error, ERROR_IO, 0, "cannot close generated output");
    }
    free(project);
    return success;
}
