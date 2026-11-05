#include "../common.c"

static void it_scopes_names_to_their_module(void) {
    setup_test();
    Symbol* a = add_local_variable(symbols, "a", "value", &error);
    Symbol* b = add_local_variable(symbols, "b", "value", &error);
    assert(a != NULL && b != NULL && a->id != b->id);
    assert(lookup_symbol(symbols, "a", "value") == a);
    assert(lookup_symbol(symbols, "missing", "value") == NULL);
    destroy_test();
}

static void it_reuses_an_existing_local(void) {
    setup_test();
    Symbol* a = add_local_variable(symbols, "main", "x", &error);
    Symbol* b = add_local_variable(symbols, "main", "x", &error);
    assert(a != NULL && a == b && symbols->count == 1);
    destroy_test();
}

static void it_preserves_defaults_and_skips_aliases_in_parameter_order(void) {
    setup_test();
    Token value = {.type = TOK_LITERAL, .lexeme = "17"};
    Token path = {.type = TOK_PATH, .lexeme = ".math.add"};
    Symbol* param = add_declarative_symbol(symbols, "main", "value", SYM_PARAM, value, &error);
    Symbol* alias = add_declarative_symbol(symbols, "main", "add", SYM_ALIAS, path, &error);
    assert(param != NULL && alias != NULL);
    assert(param->default_int == 17);
    assert(strcmp(alias->path_link, ".math.add") == 0);
    assert(get_nth_symbol(symbols, "main", 0, SYM_PARAM) == param);
    assert(get_nth_symbol(symbols, "main", 1, SYM_PARAM) == NULL);
    destroy_test();
}

static void it_rejects_duplicate_declarations_without_adding_a_symbol(void) {
    setup_test();
    Token value = {.type = TOK_LITERAL, .lexeme = "0"};
    Symbol* first = add_declarative_symbol(symbols, "main", "x", SYM_PARAM, value, &error);
    Symbol* duplicate = add_declarative_symbol(symbols, "main", "x", SYM_PARAM, value, &error);
    assert(first != NULL && duplicate == NULL);
    assert(error.code == ERROR_INVALID && symbols->count == 1);
    destroy_test();
}

static void it_rejects_variable_defaults_without_mutating_the_table(void) {
    setup_test();
    Token value = {.type = TOK_IDENT, .lexeme = "other"};
    Symbol* result = add_declarative_symbol(symbols, "main", "x", SYM_PARAM, value, &error);
    assert(result == NULL && error.code == ERROR_INVALID && symbols->count == 0);
    destroy_test();
}

static void it_keeps_tables_independent_and_can_reset_one(void) {
    setup_test();
    SymbolTable* other = calloc(1, sizeof(*other));
    assert(other != NULL);
    symbols_init(other);
    Symbol* a = add_local_variable(symbols, "main", "x", &error);
    Symbol* b = add_local_variable(other, "main", "y", &error);
    assert(a != NULL && b != NULL && a->id == b->id);
    symbols_init(symbols);
    assert(lookup_symbol(symbols, "main", "x") == NULL);
    assert(lookup_symbol(other, "main", "y") == b);
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
    printf("unit/symbols/table: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
