#include <stdint.h>
#include <string.h>
#include "options.h"

void driver_options_init(DriverOptions *options, SourceAllocator allocator) {
    *options = (DriverOptions){.compiler = "gcc"};
    source_arena_init(&options->arena, allocator);
}

void driver_options_destroy(DriverOptions *options) {
    source_arena_destroy(&options->arena);
    *options = (DriverOptions){0};
}

static bool invalid(DriverOptions *options, const char *message) {
    options->error = (Diagnostic){.code = DIAGNOSTIC_INVALID_PATH, .message = message};
    return false;
}

static bool value_option(DriverOptions *options, const char *name, const char *value,
    const char ***exclusions, size_t *excluded) {
    if (strcmp(name, "--root") == 0) {
        options->roots[options->root_count++].path = value;
    } else if (strcmp(name, "--exclude") == 0) {
        (*exclusions)[(*excluded)++] = value;
    } else if (strcmp(name, "--list-action") == 0) {
        options->libraries.action = value;
    } else if (strcmp(name, "--list-index") == 0) {
        options->libraries.index = value;
    } else if (strcmp(name, "--list-condition") == 0) {
        options->libraries.condition = value;
    } else if (strcmp(name, "--list-outcome") == 0) {
        options->libraries.outcome = value;
    } else {
        return invalid(options, "Unknown command option");
    }
    return true;
}

bool driver_options_parse(DriverOptions *options, int count, char *const arguments[]) {
    options->request = driver_parse_request(count, arguments);
    if (options->request == DRIVER_REQUEST_UNAVAILABLE || count < 4 || !module_path_valid(arguments[3])) {
        return invalid(options, "Expected a source root and an exact logical module path");
    }
    if ((size_t)count > SIZE_MAX / sizeof(ModuleRoot) || (size_t)count > SIZE_MAX / sizeof(char *)) {
        return invalid(options, "Command argument storage is unrepresentable");
    }
    options->roots = source_arena_allocate(&options->arena, (size_t)count * sizeof(*options->roots));
    const char **exclusions = source_arena_allocate(&options->arena, (size_t)count * sizeof(*exclusions));
    if (options->roots == NULL || exclusions == NULL) {
        options->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
            .message = "Command options could not allocate storage"};
        return false;
    }
    memset(options->roots, 0, (size_t)count * sizeof(*options->roots));
    options->entry = arguments[3];
    options->roots[options->root_count++].path = arguments[2];
    size_t excluded = 0;
    for (int index = 4; index < count; index += 2) {
        if (index + 1 >= count || !value_option(options, arguments[index], arguments[index + 1], &exclusions, &excluded)) {
            return options->error.message != NULL ? false : invalid(options, "Command option needs a value");
        }
    }
    for (size_t index = 0; index < options->root_count; index++) {
        options->roots[index].excluded = exclusions;
        options->roots[index].excluded_count = excluded;
    }
    options->valid = true;
    return true;
}
