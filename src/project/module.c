#include "project/internal.h"
#include "lexer/sections.h"
#include "parser/parser.h"
#include "codegen/native.h"
#include "codegen/ast.h"
#include "semantic/semantic.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

static FILE* open_module(const char* path, ModuleSections* sections, Error* error) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        error_set(error, ERROR_IO, 0, "cannot open '%s': %s", path, strerror(errno));
        return NULL;
    }
    if (!split_module_sections(file, sections, error)) {
        fclose(file);
        return NULL;
    }
    return file;
}

static bool read_module_declarations(SymbolTable* symbols, ModuleFile* module, Error* error) {
    ModuleSections sections;
    FILE* file = open_module(module->file_path, &sections, error);
    if (!file) return false;
    module->implementation = sections.implementation;
    if (module->implementation == MODULE_DECLARATION_ONLY) {
        fclose(file);
        return true;
    }
    Lexer lexer;
    bool success = init_lexer(&lexer, file, module->source_path, sections.parameters_start, sections.parameters_end,
        sections.parameters_line, error) &&
                    parse_declaration_section(&lexer, &module->syntax, AST_PARAMETER_DECLARATION) &&
                    init_lexer(&lexer, file, module->source_path, sections.results_start, sections.results_end,
                        sections.results_line, error) &&
                    parse_declaration_section(&lexer, &module->syntax, AST_RESULT_DECLARATION) &&
                    declare_module_symbols(&module->syntax, symbols, module->module_path, error);
    fclose(file);
    return success;
}

bool parse_project_declarations(Project* project, Error* error) {
    for (int index = 0; index < project->file_count; index++) {
        if (!read_module_declarations(&project->symbols, &project->files[index], error)) {
            snprintf(error->module, sizeof(error->module), "%s", project->files[index].module_path);
            return false;
        }
        ModuleFile* module = &project->files[index];
        if (module->implementation != MODULE_DECLARATION_ONLY &&
            graph_add_node(&project->graph, module->module_path, error) < 0) return false;
    }
    return true;
}

static bool read_module_tokens(const ModuleFile* module, TokenStream* stream, Error* error) {
    ModuleSections sections;
    FILE* file = open_module(module->file_path, &sections, error);
    if (!file) return false;
    Lexer lexer;
    bool success = init_lexer(&lexer, file, module->source_path, sections.code_start, sections.code_end,
        sections.code_line, error);
    if (success) success = read_token_stream(&lexer, stream);
    fclose(file);
    return success;
}

bool parse_project_bodies(Project* project, const int* dependency_order, Error* error) {
    for (int index = 0; index < project->graph.node_count; index++) {
        const char* path = graph_path(&project->graph, dependency_order[index]);
        ModuleFile* module = find_module_file(project, path);
        if (!module)
            return error_set(error, ERROR_INVALID, 0, "module '%s' has no source file", path);
        if (module->implementation == MODULE_DECLARATION_ONLY)
            return error_set(error, ERROR_INVALID, 0, "module '%s' has no implementation annotation", path);
        if (module->implementation == MODULE_C_IMPLEMENTATION) continue;
        TokenStream stream = {0};
        bool success = read_module_tokens(module, &stream, error);
        if (success) {
            StatementParser parser = {.stream = &stream, .module_path = path,
                .module = &module->syntax, .error = error};
            success = parse_module_body(&parser);
        }
        destroy_token_stream(&stream);
        if (!success) {
            snprintf(error->module, sizeof(error->module), "%s", path);
            return false;
        }
    }
    return true;
}

bool validate_project_bodies(Project* project, const int* dependency_order, Error* error) {
    /* Every body is parsed before validation; no token buffer survives this point. */
    for (int index = 0; index < project->graph.node_count; index++) {
        const char* path = graph_path(&project->graph, dependency_order[index]);
        ModuleFile* module = find_module_file(project, path);
        if (module->implementation == MODULE_C_IMPLEMENTATION) continue;
        if (!validate_module_body(&module->syntax, &project->symbols, &project->graph, path, error)) return false;
    }
    return true;
}

void destroy_project_syntax(Project* project) {
    for (int index = 0; index < project->file_count; index++) ast_destroy(&project->files[index].syntax);
}

bool emit_project_module(Project* project, const ModuleFile* module, Error* error) {
    if (module->implementation == MODULE_C_IMPLEMENTATION)
        return emit_native_wrapper(&project->emitter, module->module_path, error);
    if (!module->syntax.validated)
        return error_set(error, ERROR_INVALID, 0, "module '%s' has no validated body", module->module_path);
    emit_module_start(&project->emitter, module->module_path);
    bool success = emit_ast_body(&project->emitter, &module->syntax, error);
    emit_module_end(&project->emitter);
    return success;
}

bool validate_entry(Project* project, const char* entry_module, Error* error) {
    if (!find_module_file(project, entry_module))
        return error_set(error, ERROR_INVALID, 0, "entry '%s' has no source file", entry_module);
    if (find_module_file(project, entry_module)->implementation == MODULE_DECLARATION_ONLY)
        return error_set(error, ERROR_INVALID, 0,
            "entry module '%s' has no implementation annotation", entry_module);
    Symbol* entry_parameter = find_symbol_by_position(&project->symbols, entry_module, 0, SYMBOL_PARAMETER);
    if (entry_parameter)
        return error_set(error, ERROR_INVALID, 0,
            "entry module '%s' must take no parameters (found parameter '%s')",
            entry_module, entry_parameter->name);
    return true;
}
