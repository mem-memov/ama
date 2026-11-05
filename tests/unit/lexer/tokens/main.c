#include "../common.c"

static void it_reads_names_literals_and_module_paths(void) {
    setup_test();
    set_source("имя 42 .sys.math.add");
    Token name = get_next_token(&lexer);
    Token number = get_next_token(&lexer);
    Token path = get_next_token(&lexer);
    Token end = get_next_token(&lexer);
    assert(name.type == TOK_IDENT && strcmp(name.lexeme, "имя") == 0);
    assert(number.type == TOK_LITERAL && strcmp(number.lexeme, "42") == 0);
    assert(path.type == TOK_PATH && strcmp(path.lexeme, ".sys.math.add") == 0);
    assert(end.type == TOK_EOF);
    destroy_test();
}

static void it_tracks_comments_blank_lines_and_crlf(void) {
    setup_test();
    set_source("x\r\n# comment\r\n?\r\n\r\n y\r\n");
    Token x = get_next_token(&lexer);
    Token question = get_next_token(&lexer);
    Token y = get_next_token(&lexer);
    assert(x.line == 1 && x.is_bol);
    assert(question.type == TOK_QUESTION && !question.is_after_blank && question.line == 3);
    assert(y.indent == 1 && y.line == 5 && y.is_after_blank);
    destroy_test();
}

static void it_reports_removed_addressing_without_exiting(void) {
    setup_test();
    set_source("@x");
    Token token = get_next_token(&lexer);
    assert(token.type == TOK_ERROR && error.code == ERROR_INVALID);
    assert(error.line == 1 && strstr(error.message, "0x40") != NULL);
    destroy_test();
}

static void it_reports_a_stray_carriage_return_inside_a_word(void) {
    setup_test();
    set_source("name\rbroken");
    Token token = get_next_token(&lexer);
    assert(token.type == TOK_ERROR && error.code == ERROR_INVALID);
    destroy_test();
}

static void it_rejects_an_oversized_identifier(void) {
    setup_test();
    char text[MAX_LEXEME_LEN + 2];
    memset(text, 'x', sizeof(text) - 1);
    text[sizeof(text) - 1] = '\0';
    set_source(text);
    Token token = get_next_token(&lexer);
    assert(token.type == TOK_ERROR && error.code == ERROR_LIMIT);
    destroy_test();
}

void run(void) {
    it_reads_names_literals_and_module_paths();
    it_tracks_comments_blank_lines_and_crlf();
    it_reports_removed_addressing_without_exiting();
    it_reports_a_stray_carriage_return_inside_a_word();
    it_rejects_an_oversized_identifier();
}

int main(void) {
    run();
    printf("unit/lexer/tokens: %d test(s) passed\n", count);
    return EXIT_SUCCESS;
}
