#include "../common.c"

static void it_scopes_names_to_their_module(void) {
    setup_test();
    Symbol* first_symbol = add_local_variable(symbols, "first_module", "value", &error);
    Symbol* second_symbol = add_local_variable(symbols, "second_module", "value", &error);
    assert(first_symbol != NULL && second_symbol != NULL && first_symbol->symbol_id != second_symbol->symbol_id);
    assert(lookup_symbol(symbols, "first_module", "value") == first_symbol);
    assert(lookup_symbol(symbols, "missing", "value") == NULL);
    destroy_test();
}

static void it_reuses_an_existing_local(void) {
    setup_test();
    Symbol* first_symbol = add_local_variable(symbols, "main", "x", &error);
    Symbol* second_symbol = add_local_variable(symbols, "main", "x", &error);
    assert(first_symbol != NULL && first_symbol == second_symbol && symbols->symbol_count == 1);
    destroy_test();
}

static void it_preserves_defaults_and_skips_aliases_in_parameter_order(void) {
    setup_test();
    Token value = {.type = TOKEN_LITERAL, .lexeme = "17"};
    Token path = {.type = TOKEN_MODULE_PATH, .lexeme = ".math.add"};
    Symbol* parameter = add_declarative_symbol(symbols, "main", "value", SYMBOL_PARAMETER, value, &error);
    Symbol* alias = add_declarative_symbol(symbols, "main", "add", SYMBOL_MODULE_ALIAS, path, &error);
    assert(parameter != NULL && alias != NULL);
    assert(parameter->default_value == 17);
    assert(strcmp(alias->alias_path, ".math.add") == 0);
    assert(find_symbol_by_position(symbols, "main", 0, SYMBOL_PARAMETER) == parameter);
    assert(find_symbol_by_position(symbols, "main", 1, SYMBOL_PARAMETER) == NULL);
    destroy_test();
}

static void it_rejects_duplicate_declarations_without_adding_a_symbol(void) {
    setup_test();
    Token value = {.type = TOKEN_LITERAL, .lexeme = "0"};
    Symbol* first = add_declarative_symbol(symbols, "main", "x", SYMBOL_PARAMETER, value, &error);
    Symbol* duplicate = add_declarative_symbol(symbols, "main", "x", SYMBOL_PARAMETER, value, &error);
    assert(first != NULL && duplicate == NULL);
    assert(error.error_code == ERROR_INVALID && symbols->symbol_count == 1);
    destroy_test();
}

static void it_rejects_variable_defaults_without_mutating_the_table(void) {
    setup_test();
    Token value = {.type = TOKEN_IDENTIFIER, .lexeme = "other"};
    Symbol* result = add_declarative_symbol(symbols, "main", "x", SYMBOL_PARAMETER, value, &error);
    assert(result == NULL && error.error_code == ERROR_INVALID && symbols->symbol_count == 0);
    destroy_test();
}

static void it_keeps_tables_independent_and_can_reset_one(void) {
    setup_test();
    SymbolTable* other = calloc(1, sizeof(*other));
    assert(other != NULL);
    symbols_init(other);
    Symbol* first_symbol = add_local_variable(symbols, "main", "x", &error);
    Symbol* second_symbol = add_local_variable(other, "main", "y", &error);
    assert(first_symbol != NULL && second_symbol != NULL && first_symbol->symbol_id == second_symbol->symbol_id);
    symbols_init(symbols);
    assert(lookup_symbol(symbols, "main", "x") == NULL);
    assert(lookup_symbol(other, "main", "y") == second_symbol);
    free(other);
    destroy_test();
}

void run(void) {
    it_scopes_names_to_their_module();
    it_reuses_an_existing_local();
    it_preserves_defaults_and_skips_aliases_in_parameter_order();
    it_rejects_duplicate_declarations_without_adding_a_symbol();
    it_rejects_variable_defaults_without_mutating_the_table();
    it_keeps_tables_independent_and_can_reset_one();
}

int main(void) {
    run();
    printf("unit/symbols/table: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
