#include "project/internal.h"
#include "parser/parser.h"
#include "semantic/initializers.h"
#include <errno.h>
#include <string.h>

static bool parse_typed_body(FILE* file, const ModuleFile* module,
    const ModuleSections* sections, AstModule* syntax, Error* error) {
    Lexer lexer;
    TokenStream stream = {0};
    bool success = init_lexer(&lexer, file, module->source_path, sections->code_start,
        sections->code_end, sections->code_line, error) && read_token_stream(&lexer, &stream);
    if (success) {
        StatementParser parser = {.stream = &stream, .module_path = module->module_path,
            .module = syntax, .error = error, .typed_declarations = true};
        success = parse_module_body(&parser);
    }
    destroy_token_stream(&stream);
    return success;
}

static bool parse_typed_module(ModuleFile* module, Error* error) {
    FILE* file = fopen(module->file_path, "rb");
    if (!file)
        return error_set(error, ERROR_IO, 0, "cannot open '%s': %s", module->file_path, strerror(errno));
    ModuleSections sections;
    bool success = split_module_sections(file, &sections, error);
    if (success) module->implementation = sections.implementation;
    if (success && module->implementation != MODULE_DECLARATION_ONLY) {
        Lexer lexer;
        success = init_lexer(&lexer, file, module->source_path, sections.parameters_start,
            sections.parameters_end, sections.parameters_line, error) &&
            parse_typed_declaration_section(&lexer, &module->syntax, AST_PARAMETER_DECLARATION) &&
            init_lexer(&lexer, file, module->source_path, sections.results_start,
                sections.results_end, sections.results_line, error) &&
            parse_typed_declaration_section(&lexer, &module->syntax, AST_RESULT_DECLARATION);
        if (success && module->implementation == MODULE_AMA_IMPLEMENTATION)
            success = parse_typed_body(file, module, &sections, &module->syntax, error);
    }
    if (fclose(file) && success)
        success = error_set(error, ERROR_IO, 0, "cannot close '%s'", module->file_path);
    return success;
}

bool parse_typed_project(Project* project, Error* error) {
    int implemented_count = 0;
    for (int index = 0; index < project->file_count; index++) {
        ModuleFile* module = &project->files[index];
        if (!parse_typed_module(module, error)) {
            snprintf(error->module, sizeof(error->module), "%s", module->module_path);
            return false;
        }
        if (module->implementation != MODULE_DECLARATION_ONLY) implemented_count++;
    }
    return implemented_count != 0 || error_set(error, ERROR_INVALID, 0, "no implemented typed modules found");
}

static bool resolve_project_type(void* context, const char* path, SourceLocation location, Error* error) {
    ModuleFile* module = find_module_file(context, path);
    if (!module) return error_set(error, ERROR_INVALID, location.line, "unknown module type '%s'", path);
    if (module->implementation == MODULE_DECLARATION_ONLY)
        return error_set(error, ERROR_INVALID, location.line,
            "type module '%s' has no implementation annotation", path);
    return true;
}

bool validate_typed_project(Project* project, Error* error) {
    for (int index = 0; index < project->file_count; index++) {
        ModuleFile* module = &project->files[index];
        if (module->implementation == MODULE_DECLARATION_ONLY) continue;
        if (!validate_typed_declarations(&module->syntax, resolve_project_type, project, error)) {
            snprintf(error->module, sizeof(error->module), "%s", module->module_path);
            return false;
        }
    }
    return true;
}
