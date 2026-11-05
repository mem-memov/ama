#include "project/internal.h"
#include "lexer/sections.h"
#include "parser/parser.h"
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

static bool read_module_declarations(SymbolTable* symbols, const ModuleFile* module, Error* error) {
    ModuleSections sections;
    FILE* file = open_module(module->abs_path, &sections, error);
    if (!file) return false;
    Lexer lexer;
    bool success = init_lexer(&lexer, file, sections.params_start, sections.params_end,
                              sections.params_line, error) &&
                   parse_declaration_section(&lexer, symbols, module->rel_path, SYM_PARAM) &&
                   init_lexer(&lexer, file, sections.returns_start, sections.returns_end,
                              sections.returns_line, error) &&
                   parse_declaration_section(&lexer, symbols, module->rel_path, SYM_RETURN);
    fclose(file);
    return success;
}

bool pass1_declarations(Project* project, Error* error) {
    for (int i = 0; i < project->file_count; i++) {
        if (!read_module_declarations(&project->symbols, &project->files[i], error)) {
            snprintf(error->module, sizeof(error->module), "%s", project->files[i].rel_path);
            return false;
        }
    }
    return true;
}

static bool append_token(TokenStream* stream, int* capacity, Token token, Error* error) {
    if (stream->size == *capacity) {
        if (*capacity > INT_MAX / 2)
            return error_set(error, ERROR_LIMIT, token.line, "too many tokens");
        int next = *capacity * 2;
        Token* data = realloc(stream->data, sizeof(Token) * (size_t)next);
        if (!data) return error_set(error, ERROR_MEMORY, token.line, "out of memory (tokens)");
        stream->data = data;
        *capacity = next;
    }
    stream->data[stream->size++] = token;
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
    stream->data = malloc(sizeof(Token) * (size_t)capacity);
    if (!stream->data) success = error_set(error, ERROR_MEMORY, 0, "out of memory (tokens)");
    while (success) {
        Token token = get_next_token(&lexer);
        if (token.type == TOK_ERROR) { success = false; break; }
        success = append_token(stream, &capacity, token, error);
        if (token.type == TOK_EOF) break;
    }
    fclose(file);
    return success;
}

static bool emit_module_body(Project* project, const char* path, TokenStream* stream, Error* error) {
    Emitter* emitter = &project->emitter;
    fputs("void ", emitter->out);
    emit_module_name(emitter->out, path);
    fputs("(void) {\n", emitter->out);
    CodeParser parser = {stream, path, &project->symbols, &project->graph, emitter, error};
    while (ts_peek(stream)->type != TOK_EOF)
        if (!parse_statement(&parser, 0)) return false;
    fputs("}\n\n", emitter->out);
    return true;
}

bool pass2_module_code(Project* project, const ModuleFile* module, Error* error) {
    TokenStream stream = {0};
    bool success = read_module_tokens(module->abs_path, &stream, error) &&
                   emit_module_body(project, module->rel_path, &stream, error);
    free(stream.data);
    if (!success) snprintf(error->module, sizeof(error->module), "%s", module->rel_path);
    return success;
}

bool validate_entry(Project* project, const char* entry, Error* error) {
    if (!find_module_file(project, entry))
        return error_set(error, ERROR_INVALID, 0, "entry '%s' has no source file", entry);
    Symbol* param = get_nth_symbol(&project->symbols, entry, 0, SYM_PARAM);
    if (param)
        return error_set(error, ERROR_INVALID, 0,
                         "entry module '%s' must take no parameters (found parameter '%s')",
                         entry, param->name);
    return true;
}
