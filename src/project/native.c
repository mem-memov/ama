#include "project/internal.h"
#include "codegen/native.h"
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

bool output_directory(const char* path, char* directory, size_t capacity, Error* error) {
    const char* slash = strrchr(path, '/');
    size_t length = slash ? (size_t)(slash - path) : 1;
    if (slash == path) length = 1;
    if (length >= capacity)
        return error_set(error, ERROR_LIMIT, 0, "output directory path is too long");
    if (slash) memcpy(directory, path, length);
    else directory[0] = '.';
    directory[length] = '\0';
    return true;
}

static bool companion_path(const ModuleFile* module, char* path, size_t capacity, Error* error) {
    int length = snprintf(path, capacity, "%s.c", module->file_path);
    return (length >= 0 && (size_t)length < capacity) ||
        error_set(error, ERROR_LIMIT, 0, "native companion path is too long");
}

bool validate_generated_names(Project* project, Error* error) {
    NameRegistry registry = {0};
    bool success = true;
    for (int index = 0; success && index < project->file_count; index++) {
        const ModuleFile* module = &project->files[index];
        if (module->implementation == MODULE_DECLARATION_ONLY) continue;
        success = name_registry_register(&registry,
            source_identity_hash(module->module_path, NULL), module->module_path, NULL, error);
    }
    for (uint32_t index = 0; success && index < project->symbols.symbol_count; index++) {
        const Symbol* symbol = &project->symbols.symbols[index];
        success = name_registry_register(&registry,
            source_identity_hash(symbol->module_path, symbol->name),
            symbol->module_path, symbol->name, error);
    }
    name_registry_destroy(&registry);
    return success;
}

bool validate_native_modules(Project* project, bool require_sources, Error* error) {
    if (!validate_generated_names(project, error)) return false;
    for (int index = 0; index < project->file_count; index++) {
        ModuleFile* module = &project->files[index];
        if (module->implementation != MODULE_C_IMPLEMENTATION) continue;
        if (require_sources || module->is_standard_library) {
            char path[MAX_FILE_PATH_LENGTH];
            if (!companion_path(module, path, sizeof(path), error)) return false;
            struct stat information;
            if (stat(path, &information) || !S_ISREG(information.st_mode))
                return error_set(error, ERROR_INVALID, 0,
                    "native module '%s' requires companion C file '%s'", module->module_path, path);
        }
    }
    return true;
}

bool emit_native_headers(Project* project, const char* directory, Error* error) {
    for (int index = 0; index < project->file_count; index++) {
        ModuleFile* module = &project->files[index];
        if (module->implementation != MODULE_C_IMPLEMENTATION) continue;
        char name[MAX_FILE_PATH_LENGTH];
        char path[MAX_FILE_PATH_LENGTH];
        if (!native_function_name(module->module_path, name, sizeof(name), error)) return false;
        int length = snprintf(path, sizeof(path), "%s/%s.h", directory, name);
        if (length < 0 || (size_t)length >= sizeof(path))
            return error_set(error, ERROR_LIMIT, 0, "native header path is too long");
        FILE* header = fopen(path, "wb");
        if (!header) return error_set(error, ERROR_IO, 0, "cannot write native header '%s': %s", path, strerror(errno));
        bool success = emit_native_header(header, &project->symbols, module->module_path, error);
        if (fclose(header) && success) success = error_set(error, ERROR_IO, 0, "cannot finish native header '%s'", path);
        if (!success) return false;
    }
    return true;
}

bool create_native_stubs(Project* project, Error* error) {
    for (int index = 0; index < project->file_count; index++) {
        ModuleFile* module = &project->files[index];
        if (module->implementation != MODULE_C_IMPLEMENTATION || module->is_standard_library) continue;
        char path[MAX_FILE_PATH_LENGTH];
        if (!companion_path(module, path, sizeof(path), error)) return false;
        FILE* source = fopen(path, "wx");
        if (!source) {
            if (errno == EEXIST) {
                struct stat information;
                if (!stat(path, &information) && S_ISREG(information.st_mode)) continue;
                return error_set(error, ERROR_INVALID, 0,
                    "native companion '%s' is not a regular file", path);
            }
            return error_set(error, ERROR_IO, 0, "cannot create native stub '%s': %s", path, strerror(errno));
        }
        bool success = emit_native_stub(source, &project->symbols, module->module_path, error);
        if (fclose(source) && success) success = error_set(error, ERROR_IO, 0, "cannot finish native stub '%s'", path);
        if (!success) { unlink(path); return false; }
    }
    return true;
}

/* Stub generation may create an output directory, but never replaces a source. */
bool prepare_header_directory(const char* directory, Error* error) {
    char path[MAX_FILE_PATH_LENGTH];
    if (strlen(directory) >= sizeof(path))
        return error_set(error, ERROR_LIMIT, 0, "header directory path is too long");
    if (!*directory) return error_set(error, ERROR_INVALID, 0, "header directory must not be empty");
    strcpy(path, directory);
    for (char* character = path + 1;; character++) {
        if (*character != '/' && *character != '\0') continue;
        char separator = *character;
        *character = '\0';
        if (mkdir(path, 0777) && errno != EEXIST)
            return error_set(error, ERROR_IO, 0, "cannot create header directory '%s': %s", path, strerror(errno));
        struct stat information;
        if (stat(path, &information) || !S_ISDIR(information.st_mode))
            return error_set(error, ERROR_IO, 0, "header path '%s' is not a directory", path);
        if (!separator) break;
        *character = separator;
    }
    return true;
}
