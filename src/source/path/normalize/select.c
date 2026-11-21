/**
 * @file
 * @brief Select containing directories and qualified constants only from normalized path data.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) uses the real
 * containing directory; [CONST-05](../../../../spec/language/declarations.md#const-05)
 * fixes the final constant component without alternate filesystem probes.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns copied results.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "source/path/logical.h"
#include "source/unicode/unicode.h"

ModulePathStatus module_path_parent(SourceArena *arena, const ModulePathValue *module,
    const ModulePathValue **output, SourceIssue *issue) {
    if (module == NULL || !module_path_canonical((const unsigned char *)module->text, module->length)) {
        return path_normalization_failure(issue, module == NULL ? (SourceSpan){0} : module->expression,
            MODULE_PATH_INVALID, "Containing directory requires a complete module", "a canonical complete module identity");
    }
    return path_normalization_store(arena, module->root, module->parts, module->count - 1,
        module->expression, output, issue);
}

ModulePathStatus module_path_constant(SourceArena *arena, const ModulePathValue *path,
    ModuleConstantPath *output, SourceIssue *issue) {
    if (path == NULL || path->count < 2) {
        return path_normalization_failure(issue, path == NULL ? (SourceSpan){0} : path->expression,
            MODULE_PATH_INVALID, "Qualified constant has no complete module prefix", "a module followed by a constant name");
    }
    ModulePathPart member = path->parts[path->count - 1];
    size_t name_error;
    if (!unicode_name(member.bytes, member.length, &name_error)) {
        return path_normalization_failure(issue, member.origin.source == NULL ? path->expression : member.origin,
            MODULE_PATH_INVALID, "Invalid constant member name", "one complete ordinary name");
    }
    size_t prefix_length = path->length - member.length - 1;
    if (!module_path_canonical((const unsigned char *)path->text, prefix_length)) {
        return path_normalization_failure(issue, path->expression, MODULE_PATH_INVALID,
            "Qualified constant prefix is not a complete module", "a complete module followed by one constant name");
    }
    const ModulePathValue *module;
    ModulePathStatus status = path_normalization_store(arena, path->root, path->parts, path->count - 1,
        path->expression, &module, issue);
    if (status != MODULE_PATH_OK) { return status; }
    if (member.length == SIZE_MAX) {
        return path_normalization_failure(issue, path->expression, MODULE_PATH_LIMIT,
            "Constant name length is not representable", "a representable constant name");
    }
    unsigned char *name = source_arena_allocate(arena, member.length + 1);
    if (name == NULL) {
        return path_normalization_failure(issue, path->expression, MODULE_PATH_OUT_OF_MEMORY,
            "Constant name allocation failed", "available constant-name storage");
    }
    memcpy(name, member.bytes, member.length);
    name[member.length] = 0;
    *output = (ModuleConstantPath){module, {name, member.length, member.origin}, path->expression};
    return MODULE_PATH_OK;
}
