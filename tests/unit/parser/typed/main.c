#include "../common.c"
#include "semantic/initializers.h"

static bool parse_typed_body(const char* text) {
    FILE* source = text_file(text);
    Lexer lexer;
    bool ready = init_lexer(&lexer, source, "typed.ama", 0, (long)strlen(text), 1, &error);
    assert(ready);
    TokenStream stream = {0};
    bool success = read_token_stream(&lexer, &stream);
    close_file(source);
    StatementParser parser = {.stream = &stream, .module_path = "main", .module = &syntax,
        .error = &error, .typed_declarations = true};
    if (success) success = parse_module_body(&parser);
    destroy_token_stream(&stream);
    return success;
}

static bool resolve_type(void* context, const char* path, SourceLocation location, Error* diagnostic) {
    return graph_has_node(context, path) ||
        error_set(diagnostic, ERROR_INVALID, location.line, "unknown module type '%s'", path);
}

static void declare_types(void) {
    int number = graph_add_node(graph, "std/integer", &error);
    int text = graph_add_node(graph, "std/text", &error);
    int item = graph_add_node(graph, "shop/item", &error);
    int unicode = graph_add_node(graph, "\u6a21\u5757/\u7c7b\u578b", &error);
    assert(number >= 0 && text >= 0 && item >= 0 && unicode >= 0);
}

static void it_resolves_owned_reference_and_list_initializers_in_declaration_order(void) {
    setup_test();
    declare_types();
    bool parsed = parse_typed_body("quantity\n.std.integer\n |255\ncopy\n.std.integer\n quantity\n"
        "item\n.shop.item\nreference&\n.shop.item\n &item\nother&\n.shop.item\n reference&\n"
        "items[]\n.shop.item\n item\nreferences&[]\n.shop.item\n &item\n reference&\n");
    assert(parsed && !syntax.validated && symbols->symbol_count == 0);
    bool checked = validate_typed_declarations(&syntax, resolve_type, graph, &error);
    assert(checked && syntax.declarations_validated && !syntax.validated);
    AstDeclaration* quantity = syntax.body.first->as.declaration;
    AstDeclaration* copy = syntax.body.first->next->as.declaration;
    assert(quantity->storage == AST_OWNED && strcmp(quantity->resolved_type, "std/integer") == 0);
    assert(quantity->initializers->value.literal_length == 3);
    assert(memcmp(quantity->initializers->value.literal_bytes, "255", 3) == 0);
    assert(quantity->initializers->value.location.line == 3 && quantity->initializers->value.location.column == 2);
    assert(copy->initializers->source == quantity);
    AstStatement* last = syntax.body.last;
    assert(last->as.declaration->storage == AST_REFERENCE_LIST);
    assert(last->as.declaration->initializers->acquires_reference);
    assert(last->as.declaration->initializers->next->source->storage == AST_REFERENCE);
    long output_position = ftell(output);
    assert(output_position == 0);
    destroy_test();
}

static void it_preserves_line_literal_bytes_after_releasing_the_token_stream(void) {
    setup_test();
    declare_types();
    char text[4096];
    const char* prefix = "lines[]\n.std.text\n |";
    size_t length = strlen(prefix);
    memcpy(text, prefix, length);
    memset(text + length, 'x', 1500);
    length += 1500;
    const char* suffix = "\n |\n ||#\"\\\t  \n";
    memcpy(text + length, suffix, strlen(suffix) + 1);
    bool parsed = parse_typed_body(text);
    memset(text, '?', sizeof(text));
    assert(parsed);
    bool checked = validate_typed_declarations(&syntax, resolve_type, graph, &error);
    assert(checked);
    const AstInitializer* first = syntax.body.first->as.declaration->initializers;
    assert(first->value.literal_length == 1500 && first->value.literal_bytes[1499] == 'x');
    assert(first->next->value.literal_length == 0);
    const char* expected = "|#\"\\\t  ";
    assert(first->next->next->value.literal_length == strlen(expected));
    assert(memcmp(first->next->next->value.literal_bytes, expected, strlen(expected)) == 0);
    destroy_test();
}

static void it_rejects_invalid_storage_and_type_line_syntax(void) {
    setup_test();
    const char* invalid[] = {
        "item &\n.shop.item\n", "item[ ]\n.shop.item\n", "&item\n.shop.item\n",
        "item[]&\n.shop.item\n", "item\n&.shop.item\n", "item\n.shop.item[]\n",
        "item\n.shop..item\n", "item\n.123\n", "item\n.shop.item\n 255\n",
        "item&\n.shop.item\n &other&\n", "item\n.shop.item\n value extra\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        ast_destroy(&syntax);
        error_clear(&error);
        bool parsed = parse_typed_body(invalid[index]);
        assert(!parsed && error.error_code == ERROR_INVALID);
    }
    destroy_test();
}

static void it_rejects_invalid_initializer_dependencies_and_nominal_types(void) {
    setup_test();
    declare_types();
    const char* invalid[] = {
        "first\n.std.integer\n later\nlater\n.std.integer\n",
        "self\n.std.integer\n self\n",
        "number\n.std.integer\ntext\n.std.text\n number\n",
        "item&\n.shop.item\n |forged\n",
        "item\n.shop.item\nreference&\n.shop.item\n &item\ncopy\n.shop.item\n reference&\n",
        "item\n.shop.item\nreference&\n.shop.item\n &item\ncopy&\n.shop.item\n &reference\n",
        "first[]\n.std.integer\nsecond[]\n.std.integer\n first[]\n",
        "scalar\n.std.integer\n |1\n |2\n",
        "item\n.shop.item\nitem&\n.shop.item\n",
        "item\n.unknown.type\n"
    };
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        ast_destroy(&syntax);
        error_clear(&error);
        bool parsed = parse_typed_body(invalid[index]);
        assert(parsed);
        bool checked = validate_typed_declarations(&syntax, resolve_type, graph, &error);
        assert(!checked && !syntax.declarations_validated && error.error_code == ERROR_INVALID);
    }
    destroy_test();
}

static void it_parses_typed_interface_sections_without_binding_them_as_aliases(void) {
    setup_test();
    declare_types();
    const char* text = "count\n.std.integer\n |7\n";
    FILE* source = text_file(text);
    Lexer lexer;
    bool ready = init_lexer(&lexer, source, "interface.ama", 0, (long)strlen(text), 1, &error);
    assert(ready);
    bool parsed = parse_typed_declaration_section(&lexer, &syntax, AST_PARAMETER_DECLARATION);
    close_file(source);
    assert(parsed && syntax.declarations->kind == AST_PARAMETER_DECLARATION);
    bool checked = validate_typed_declarations(&syntax, resolve_type, graph, &error);
    assert(checked && syntax.declarations_validated && !syntax.validated);
    bool bound = declare_module_symbols(&syntax, symbols, "main", &error);
    assert(!bound && symbols->symbol_count == 0 && strstr(error.message, "module-value ABI") != NULL);
    destroy_test();
}

static void it_distinguishes_declaration_initialization_from_indented_assignment(void) {
    setup_test();
    declare_types();
    bool parsed = parse_typed_body("first\n.std.integer\n |1\nsecond\n.std.integer\n |2\nfirst\n second\n");
    assert(parsed && syntax.body.last->kind == AST_ASSIGNMENT);
    assert(strcmp(syntax.body.last->as.assignment.source.text, "second") == 0);
    bool checked = validate_typed_declarations(&syntax, resolve_type, graph, &error);
    assert(checked);
    bool numeric = validate_module_body(&syntax, symbols, graph, "main", &error);
    assert(!numeric && !syntax.validated && strstr(error.message, "module-value ABI") != NULL);
    ast_destroy(&syntax);
    error_clear(&error);
    bool legacy = parse_text("value\n1\n");
    assert(legacy && syntax.validated);
    destroy_test();
}

static void it_keeps_unicode_type_identity_and_rejects_literals_outside_declarations(void) {
    setup_test();
    declare_types();
    bool parsed = parse_typed_body("\u53d8\u91cf\n.\u6a21\u5757.\u7c7b\u578b\n |\u4f60\u597d\n");
    assert(parsed);
    bool checked = validate_typed_declarations(&syntax, resolve_type, graph, &error);
    assert(checked && strcmp(syntax.body.first->as.declaration->resolved_type, "\u6a21\u5757/\u7c7b\u578b") == 0);
    const char* invalid[] = {"value\n.std.integer\n |1\nvalue\n |2\n", "worker!\n < 2\n"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]); index++) {
        ast_destroy(&syntax);
        error_clear(&error);
        parsed = parse_typed_body(invalid[index]);
        assert(!parsed && strstr(error.message, "literals are only allowed") != NULL);
    }
    destroy_test();
}

void run(void) {
    it_resolves_owned_reference_and_list_initializers_in_declaration_order();
    it_preserves_line_literal_bytes_after_releasing_the_token_stream();
    it_rejects_invalid_storage_and_type_line_syntax();
    it_rejects_invalid_initializer_dependencies_and_nominal_types();
    it_parses_typed_interface_sections_without_binding_them_as_aliases();
    it_distinguishes_declaration_initialization_from_indented_assignment();
    it_keeps_unicode_type_identity_and_rejects_literals_outside_declarations();
}

int main(void) {
    run();
    printf("unit/parser/typed: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
