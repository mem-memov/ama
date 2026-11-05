#include "project/internal.h"
#include "lexer/sections.h"
#include "parser/parser.h"
#include "codegen/native.h"
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
    bool success = init_lexer(&lexer, file, sections.parameters_start, sections.parameters_end,
        sections.parameters_line, error) &&
                    parse_declaration_section(&lexer, symbols, module->module_path, SYMBOL_PARAMETER) &&
                    init_lexer(&lexer, file, sections.results_start, sections.results_end,
                        sections.results_line, error) &&
                    parse_declaration_section(&lexer, symbols, module->module_path, SYMBOL_RESULT);
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

static bool append_token(TokenStream* stream, int* capacity, Token token, Error* error) {
    if (stream->token_count == *capacity) {
        if (*capacity > INT_MAX / 2)
            return error_set(error, ERROR_LIMIT, token.line_number, "too many tokens");
        int next_capacity = *capacity * 2;
        Token* tokens = realloc(stream->tokens, sizeof(Token) * (size_t)next_capacity);
        if (!tokens) return error_set(error, ERROR_MEMORY, token.line_number, "out of memory (tokens)");
        stream->tokens = tokens;
        *capacity = next_capacity;
    }
    stream->tokens[stream->token_count++] = token;
    return true;
}

static bool read_module_tokens(const char* path, TokenStream* stream, Error* error) {
    ModuleSections sections;
    FILE* file = open_module(path, &sections, error);
    if (!file) return false;
    Lexer lexer;
    bool success = init_lexer(&lexer, file, sections.code_start, sections.code_end,
        sections.code_line, error);
    int capacity = 1024;
    stream->tokens = malloc(sizeof(Token) * (size_t)capacity);
    if (!stream->tokens) success = error_set(error, ERROR_MEMORY, 0, "out of memory (tokens)");
    while (success) {
        Token token = get_next_token(&lexer);
        if (token.type == TOKEN_ERROR) { success = false; break; }
        success = append_token(stream, &capacity, token, error);
        if (token.type == TOKEN_END_OF_FILE) break;
    }
    fclose(file);
    return success;
}

static bool emit_module_body(Project* project, const char* path, TokenStream* stream, Error* error) {
    Emitter* emitter = &project->emitter;
    emit_module_start(emitter, path);
    StatementParser parser = {stream, path, &project->symbols, &project->graph, emitter, error};
    while (token_stream_peek(stream)->type != TOKEN_END_OF_FILE)
        if (!parse_statement(&parser, 0)) return false;
    emit_module_end(emitter);
    return true;
}

bool emit_project_module(Project* project, const ModuleFile* module, Error* error) {
    if (module->implementation == MODULE_DECLARATION_ONLY)
        return error_set(error, ERROR_INVALID, 0,
            "module '%s' has no implementation annotation", module->module_path);
    if (module->implementation == MODULE_C_IMPLEMENTATION)
        return emit_native_wrapper(&project->emitter, module->module_path, error);
    TokenStream stream = {0};
    bool success = read_module_tokens(module->file_path, &stream, error) &&
                    emit_module_body(project, module->module_path, &stream, error);
    free(stream.tokens);
    if (!success) snprintf(error->module, sizeof(error->module), "%s", module->module_path);
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
