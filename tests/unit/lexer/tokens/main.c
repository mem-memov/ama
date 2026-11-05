#include "../common.c"

static void it_reads_names_literals_and_module_paths(void) {
    setup_test();
    set_source("\u03bbidentifier 42 .sys.math.add");
    Token name = get_next_token(&lexer);
    Token number = get_next_token(&lexer);
    Token path = get_next_token(&lexer);
    Token end_of_file_token = get_next_token(&lexer);
    assert(name.type == TOKEN_IDENTIFIER && strcmp(name.lexeme, "\u03bbidentifier") == 0);
    assert(number.type == TOKEN_LITERAL && strcmp(number.lexeme, "42") == 0);
    assert(path.type == TOKEN_MODULE_PATH && strcmp(path.lexeme, ".sys.math.add") == 0);
    assert(end_of_file_token.type == TOKEN_END_OF_FILE);
    destroy_test();
}

static void it_tracks_comments_blank_lines_and_crlf(void) {
    setup_test();
    set_source("x\r\n# comment\r\n?\r\n\r\n y\r\n");
    Token condition_token = get_next_token(&lexer);
    Token question = get_next_token(&lexer);
    Token value_token = get_next_token(&lexer);
    assert(condition_token.line_number == 1 && condition_token.is_at_line_start);
    assert(question.type == TOKEN_QUESTION_MARK && !question.has_preceding_blank_line && question.line_number == 3);
    assert(value_token.indentation == 1 && value_token.line_number == 5 && value_token.has_preceding_blank_line);
    destroy_test();
}

static void it_reports_removed_addressing_without_exiting(void) {
    setup_test();
    set_source("@x");
    Token token = get_next_token(&lexer);
    assert(token.type == TOKEN_ERROR && error.error_code == ERROR_INVALID);
    assert(error.line_number == 1 && strstr(error.message, "0x40") != NULL);
    destroy_test();
}

static void it_reports_a_stray_carriage_return_inside_a_word(void) {
    setup_test();
    set_source("name\rbroken");
    Token token = get_next_token(&lexer);
    assert(token.type == TOKEN_ERROR && error.error_code == ERROR_INVALID);
    destroy_test();
}

static void it_rejects_an_oversized_identifier(void) {
    setup_test();
    char text[MAX_TOKEN_TEXT_LENGTH + 2];
    memset(text, 'x', sizeof(text) - 1);
    text[sizeof(text) - 1] = '\0';
    set_source(text);
    Token token = get_next_token(&lexer);
    assert(token.type == TOKEN_ERROR && error.error_code == ERROR_LIMIT);
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
    printf("unit/lexer/tokens: %d test(s) passed\n", test_count);
    return EXIT_SUCCESS;
}
