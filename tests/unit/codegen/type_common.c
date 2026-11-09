#ifndef AMA_TESTS_CODEGEN_TYPE_COMMON_C
#define AMA_TESTS_CODEGEN_TYPE_COMMON_C

#include "../common.c"
#include "codegen/types.h"
#include "common/names.h"
#include "parser/parser.h"
#include "semantic/initializers.h"

static AstModule syntax[5];
static TypeModule types[5];
static size_t type_count;
static FILE* header;
static FILE* implementation;

static void setup_test(void) {
    test_count++;
    error_clear(&error);
    memset(syntax, 0, sizeof(syntax));
    memset(types, 0, sizeof(types));
    type_count = 0;
    header = tmpfile();
    implementation = tmpfile();
    assert(header && implementation);
}

static void destroy_test(void) {
    close_file(header);
    close_file(implementation);
    for (size_t index = 0; index < 5; index++) ast_destroy(&syntax[index]);
}

static bool resolve_type(void* context, const char* path, SourceLocation location, Error* diagnostic) {
    (void)context;
    for (size_t index = 0; index < type_count; index++)
        if (strcmp(types[index].module_path, path) == 0) return true;
    return error_set(diagnostic, ERROR_INVALID, location.line, "unknown test type '%s'", path);
}

static void parse_body(size_t index, const char* text) {
    FILE* input = text_file(text);
    Lexer lexer;
    assert(init_lexer(&lexer, input, "\u6a21\u5757.ama", 0, (long)strlen(text), 1, &error));
    TokenStream stream = {0};
    assert(read_token_stream(&lexer, &stream));
    StatementParser parser = {.stream = &stream, .module = &syntax[index],
        .module_path = types[index].module_path, .error = &error, .typed_declarations = true};
    assert(parse_module_body(&parser));
    destroy_token_stream(&stream);
    close_file(input);
}

static void parse_interface(size_t index, AstDeclarationKind kind, const char* text) {
    FILE* input = text_file(text);
    Lexer lexer;
    assert(init_lexer(&lexer, input, "interface.ama", 0, (long)strlen(text), 1, &error));
    assert(parse_typed_declaration_section(&lexer, &syntax[index], kind));
    close_file(input);
}

static void validate_types(void) {
    for (size_t index = 0; index < type_count; index++)
        if (types[index].module)
            assert(validate_typed_declarations(&syntax[index], resolve_type, NULL, &error));
}

static bool generate_types(void) {
    return emit_type_layouts(header, implementation, "types.h", types, type_count, &error);
}

static void expect_no_output(void) {
    assert(ftell(header) == 0 && ftell(implementation) == 0);
}

#endif
