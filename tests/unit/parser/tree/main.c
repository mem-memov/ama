#include "../common.c"

static void it_parses_unresolved_structure_without_symbols_or_c_output(void) {
    setup_test();
    bool parsed = parse_syntax_at("x\n42\nx\n?\n >\n:\n x\n {\n  ^\n }\n", "tree.ama");
    assert(parsed && symbols->symbol_count == 0 && !syntax.validated);
    long output_position = ftell(output);
    assert(output_position == 0);
    const AstStatement* assignment = syntax.body.first;
    const AstStatement* branch = assignment->next;
    assert(assignment->kind == AST_ASSIGNMENT && branch->kind == AST_BRANCH);
    assert(assignment->as.assignment.receiver.symbol_id == AST_UNRESOLVED_SYMBOL);
    assert(strcmp(assignment->as.assignment.source.text, "42") == 0);
    assert(assignment->location.line == 1 && strcmp(assignment->location.path, "tree.ama") == 0);
    assert(branch->as.control.body.first->kind == AST_RETURN);
    assert(branch->as.control.has_alternative && branch->as.control.alternative_location.line == 6);
    const AstStatement* loop = branch->as.control.alternative.first;
    assert(loop->kind == AST_LOOP && loop->as.control.body.first->kind == AST_BREAK);
    assert(loop->as.control.end_location.line == 10);
    destroy_test();
}

static void it_preserves_declarations_and_call_arguments_before_resolution(void) {
    setup_test();
    FILE* declarations = text_file("add\n.math.add\n");
    Lexer lexer;
    bool ready = init_lexer(&lexer, declarations, "interface.ama", 0, 14, 1, &error);
    assert(ready);
    bool parsed = parse_declaration_section(&lexer, &syntax, AST_PARAMETER_DECLARATION);
    close_file(declarations);
    assert(parsed && syntax.declarations->name.symbol_id == AST_UNRESOLVED_SYMBOL);
    assert(syntax.declarations->value.kind == AST_PATH_VALUE);
    parsed = parse_syntax_at("add!\n left < unknown\n right < 7\n sum > answer\n", "caller.ama");
    assert(parsed && symbols->symbol_count == 0 && !syntax.validated);
    const AstStatement* call = syntax.body.first;
    assert(call->kind == AST_CALL && call->as.call.target.symbol_id == AST_UNRESOLVED_SYMBOL);
    const AstArgument* first = call->as.call.arguments;
    assert(first->is_named && first->kind == AST_PARAMETER_ARGUMENT && strcmp(first->value.text, "unknown") == 0);
    assert(first->callee_symbol_id == AST_UNRESOLVED_SYMBOL && first->location.line == 2);
    assert(strcmp(first->next->label.text, "right") == 0);
    assert(first->next->next->kind == AST_RESULT_ARGUMENT && strcmp(first->next->next->value.text, "answer") == 0);
    destroy_test();
}

static void it_rejects_semantic_errors_before_generation(void) {
    setup_test();
    bool parsed = parse_syntax_at("destination\nmissing\n", "invalid.ama");
    assert(parsed && error.error_code == ERROR_NONE);
    bool validated = validate_module_body(&syntax, symbols, graph, "main", &error);
    assert(!validated && !syntax.validated && strstr(error.message, "unknown numeric name") != NULL);
    assert(error.line_number == 2);
    error_clear(&error);
    bool emitted = emit_ast_body(&emitter, &syntax, &error);
    assert(!emitted && strstr(error.message, "unvalidated AST") != NULL);
    long output_position = ftell(output);
    assert(output_position == 0);
    destroy_test();
}

static void it_releases_partial_trees_and_can_parse_again(void) {
    setup_test();
    bool parsed = parse_syntax_at("x\n{\n y\n", "broken.ama");
    assert(!parsed && syntax.allocations != NULL);
    ast_destroy(&syntax);
    assert(syntax.allocations == NULL && syntax.body.first == NULL);
    error_clear(&error);
    parsed = parse_text("value\n3\n");
    assert(parsed && syntax.validated);
    destroy_test();
}

static void it_bounds_nesting_before_exhausting_the_c_stack(void) {
    setup_test();
    char text[65536];
    size_t length = 0;
    for (unsigned int level = 0; level <= MAX_AST_NESTING; level++) {
        memset(text + length, ' ', level);
        length += level;
        memcpy(text + length, "x\n", 2);
        length += 2;
        memset(text + length, ' ', level);
        length += level;
        memcpy(text + length, "?\n", 2);
        length += 2;
    }
    text[length] = '\0';
    bool parsed = parse_syntax_at(text, "deep.ama");
    assert(!parsed && strstr(error.message, "statement nesting exceeds") != NULL);
    destroy_test();
}

void run(void) {
    it_parses_unresolved_structure_without_symbols_or_c_output();
    it_preserves_declarations_and_call_arguments_before_resolution();
    it_rejects_semantic_errors_before_generation();
    it_releases_partial_trees_and_can_parse_again();
    it_bounds_nesting_before_exhausting_the_c_stack();
}

int main(void) {
    run();
    printf("unit/parser/tree: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
