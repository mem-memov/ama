#include "project/internal.h"
#include "lexer/sections.h"
#include "parser/parser.h"
#include "symbols/table.h"
#include "codegen/emitter.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

/* Both passes use the same section boundaries and physical source line numbers. */
static FILE* open_module(const char* path, ModuleSections* sections) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "error: cannot open '%s': %s\n", path, strerror(errno));
        exit(1);
    }
    if (!split_module_sections(file, sections)) {
        fclose(file);
        exit(1);
    }
    return file;
}

static void read_module_declarations(const ModuleFile* module) {
    ModuleSections sections;
    FILE* file = open_module(module->abs_path, &sections);
    Lexer lexer;
    init_lexer(&lexer, file, sections.params_start, sections.params_end,
               sections.params_line);
    parse_declaration_section(&lexer, module->rel_path, SYM_PARAM);
    init_lexer(&lexer, file, sections.returns_start, sections.returns_end,
               sections.returns_line);
    parse_declaration_section(&lexer, module->rel_path, SYM_RETURN);
    fclose(file);
}

void pass1_declarations(void) {
    for (int i = 0; i < G_FileCount; i++)
        read_module_declarations(&G_Files[i]);
}

static void grow_token_buffer(TokenStream* stream, int* capacity) {
    *capacity *= 2;
    Token* tokens = realloc(stream->data, sizeof(Token) * (size_t)*capacity);
    if (!tokens) {
        free(stream->data);
        fprintf(stderr, "error: out of memory (token buffer grow)\n");
        exit(1);
    }
    stream->data = tokens;
}

/* Owns its input file; the caller owns the returned, EOF-terminated token array. */
static TokenStream read_module_tokens(const char* path) {
    ModuleSections sections;
    FILE* file = open_module(path, &sections);
    Lexer lexer;
    init_lexer(&lexer, file, sections.code_start, sections.code_end, sections.code_line);
    int capacity = 1024;
    TokenStream stream = {malloc(sizeof(Token) * (size_t)capacity), 0, 0};
    if (!stream.data) {
        fclose(file);
        fprintf(stderr, "error: out of memory (token buffer)\n");
        exit(1);
    }
    for (;;) {
        Token token = get_next_token(&lexer);
        if (token.type == TOK_ERROR) {
            free(stream.data);
            fclose(file);
            exit(1);
        }
        if (stream.size == capacity)
            grow_token_buffer(&stream, &capacity);
        stream.data[stream.size++] = token;
        if (token.type == TOK_EOF)
            break;
    }
    fclose(file);
    return stream;
}

static void emit_module_body(const char* path, TokenStream* stream) {
    fprintf(G_Out, "void ");
    emit_module_name(G_Out, path);
    fprintf(G_Out, "(void) {\n");
    CodeParser parser = {stream, path};
    while (ts_peek(stream)->type != TOK_EOF)
        parse_statement(&parser, 0);
    fprintf(G_Out, "}\n\n");
}

void pass2_module_code(const char* rel_path, const char* abs_path) {
    TokenStream stream = read_module_tokens(abs_path);
    emit_module_body(rel_path, &stream);
    free(stream.data);
}

void validate_entry(const char* entry) {
    if (!find_module_file(entry)) {
        fprintf(stderr,
                "error: entry '%s' has no source file under the project "
                "root\n", entry);
        exit(1);
    }

    for (uint32_t i = 0; i < G_Table.count; i++) {
        Symbol* s = &G_Table.symbols[i];
        if (strcmp(s->module_path, entry) == 0 && s->kind == SYM_PARAM) {
            fprintf(stderr,
                    "error: entry module '%s' must take no parameters "
                    "(found parameter '%s')\n",
                    entry, s->name);
            exit(1);
        }
    }
}
