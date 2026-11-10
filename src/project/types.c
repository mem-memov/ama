#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include "project/internal.h"
#include "codegen/types.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    TypeModule modules[MAX_MODULE_FILES];
    char native_headers[MAX_MODULE_FILES][MAX_FILE_PATH_LENGTH];
    size_t module_count;
} ProjectTypes;

static bool resolve_native_header(const ModuleFile* module, char* header, Error* error) {
    char path[MAX_FILE_PATH_LENGTH];
    int length = snprintf(path, sizeof(path), "%s.h", module->file_path);
    if (length < 0 || (size_t)length >= sizeof(path))
        return error_set(error, ERROR_LIMIT, 0, "native type header path is too long");
    struct stat information;
    if (stat(path, &information) || !S_ISREG(information.st_mode))
        return error_set(error, ERROR_INVALID, 0,
            "native type '%s' requires layout header '%s'", module->module_path, path);
    char* absolute = realpath(path, NULL);
    if (!absolute) return error_set(error, ERROR_IO, 0, "cannot resolve native type header '%s'", path);
    bool success = strlen(absolute) < MAX_FILE_PATH_LENGTH;
    if (success) strcpy(header, absolute);
    free(absolute);
    return success || error_set(error, ERROR_LIMIT, 0, "native type header path is too long");
}

static int compare_type_modules(const void* left, const void* right) {
    const TypeModule* first = left;
    const TypeModule* second = right;
    return strcmp(first->module_path, second->module_path);
}

static bool collect_project_types(Project* project, ProjectTypes* types, Error* error) {
    for (int index = 0; index < project->file_count; index++) {
        ModuleFile* module = &project->files[index];
        if (module->implementation == MODULE_DECLARATION_ONLY) continue;
        size_t position = types->module_count++;
        TypeModule* type = &types->modules[position];
        type->module_path = module->module_path;
        type->module = &module->syntax;
        if (module->implementation == MODULE_C_IMPLEMENTATION) {
            if (!resolve_native_header(module, types->native_headers[position], error)) return false;
            type->native_header = types->native_headers[position];
        }
    }
    qsort(types->modules, types->module_count, sizeof(*types->modules), compare_type_modules);
    return true;
}

static bool copy_type_output(FILE* source, const char* path, Error* error) {
    if (fflush(source) || fseek(source, 0, SEEK_SET))
        return error_set(error, ERROR_IO, 0, "cannot rewind generated type output");
    FILE* destination = fopen(path, "wb");
    if (!destination) return error_set(error, ERROR_IO, 0, "cannot write '%s': %s", path, strerror(errno));
    char bytes[4096];
    size_t length;
    bool success = true;
    while ((length = fread(bytes, 1, sizeof(bytes), source))) {
        if (fwrite(bytes, 1, length, destination) != length) {
            success = false;
            break;
        }
    }
    if (ferror(source)) success = false;
    if (fclose(destination)) success = false;
    return success || error_set(error, ERROR_IO, 0, "cannot finish generated type file '%s'", path);
}

static bool make_output_paths(const char* directory, char* header, char* implementation, Error* error) {
    int header_length = snprintf(header, MAX_FILE_PATH_LENGTH, "%s/ama_types.h", directory);
    int implementation_length = snprintf(implementation, MAX_FILE_PATH_LENGTH, "%s/ama_types.c", directory);
    return (header_length >= 0 && header_length < MAX_FILE_PATH_LENGTH &&
        implementation_length >= 0 && implementation_length < MAX_FILE_PATH_LENGTH) ||
        error_set(error, ERROR_LIMIT, 0, "generated type output path is too long");
}

static bool protect_type_sources(const Project* project, const char* path, Error* error) {
    struct stat destination;
    if (stat(path, &destination)) {
        if (errno != ENOENT)
            return error_set(error, ERROR_IO, 0, "cannot inspect type output '%s': %s", path, strerror(errno));
        struct stat link_information;
        if (!lstat(path, &link_information))
            return error_set(error, ERROR_INVALID, 0, "type output '%s' is a dangling symbolic link", path);
        return true;
    }
    if (!S_ISREG(destination.st_mode))
        return error_set(error, ERROR_INVALID, 0, "type output '%s' is not a regular file", path);
    for (int index = 0; index < project->file_count; index++) {
        const ModuleFile* module = &project->files[index];
        for (int companion = 0; companion < 3; companion++) {
            const char* suffix = companion == 0 ? "" : companion == 1 ? ".c" : ".h";
            char source[MAX_FILE_PATH_LENGTH + 3];
            snprintf(source, sizeof(source), "%s%s", module->file_path, suffix);
            struct stat information;
            if (!stat(source, &information) && information.st_dev == destination.st_dev &&
                information.st_ino == destination.st_ino)
                return error_set(error, ERROR_INVALID, 0, "type output would overwrite source '%s'", source);
        }
    }
    return true;
}

static bool validate_type_outputs(const Project* project, const char* header,
    const char* implementation, Error* error) {
    if (!protect_type_sources(project, header, error) ||
        !protect_type_sources(project, implementation, error)) return false;
    struct stat header_information, implementation_information;
    if (!stat(header, &header_information) && !stat(implementation, &implementation_information) &&
        header_information.st_dev == implementation_information.st_dev &&
        header_information.st_ino == implementation_information.st_ino)
        return error_set(error, ERROR_INVALID, 0, "type header and implementation must be different files");
    return true;
}

static bool export_project_types(Project* project, ProjectTypes* types, const char* directory, Error* error) {
    FILE* header = tmpfile();
    FILE* implementation = tmpfile();
    if (!header || !implementation) {
        if (header) fclose(header);
        if (implementation) fclose(implementation);
        return error_set(error, ERROR_IO, 0, "cannot create temporary type output");
    }
    char header_path[MAX_FILE_PATH_LENGTH], implementation_path[MAX_FILE_PATH_LENGTH];
    bool success = emit_type_layouts(header, implementation, "ama_types.h",
        types->modules, types->module_count, error) &&
        prepare_header_directory(directory, error) &&
        make_output_paths(directory, header_path, implementation_path, error) &&
        validate_type_outputs(project, header_path, implementation_path, error) &&
        copy_type_output(header, header_path, error) && copy_type_output(implementation, implementation_path, error);
    if (fclose(header) && success) success = error_set(error, ERROR_IO, 0, "cannot close temporary type header");
    if (fclose(implementation) && success) success = error_set(error, ERROR_IO, 0, "cannot close temporary type source");
    return success;
}

bool generate_type_layouts_with_library(const char* project_root, const char* directory,
    const char* library_directory, Error* error) {
    error_clear(error);
    Project* project = calloc(1, sizeof(*project));
    ProjectTypes* types = calloc(1, sizeof(*types));
    if (!project || !types) {
        free(project);
        free(types);
        return error_set(error, ERROR_MEMORY, 0, "out of memory (typed project)");
    }
    bool success = scan_dir(project, project_root, "", error) &&
        load_standard_library(project, library_directory, error) &&
        parse_typed_project(project, error) && validate_typed_project(project, error) &&
        validate_native_modules(project, true, error) && collect_project_types(project, types, error) &&
        export_project_types(project, types, directory, error);
    destroy_project_syntax(project);
    free(types);
    free(project);
    return success;
}

bool generate_type_layouts(const char* project_root, const char* directory, Error* error) {
    return generate_type_layouts_with_library(project_root, directory, NULL, error);
}
