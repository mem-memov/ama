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

static void it_tracks_byte_columns_across_unicode_comments_and_crlf(void) {
    setup_test();
    const char* path = "source/\u5546\u54c1";
    const char* text = "  \u03bb 42\r\n# comment\r\n next > result\n# end";
    source = text_file(text);
    bool ready = init_lexer(&lexer, source, path, 0, (long)strlen(text), 7, &error);
    assert(ready);
    Token name = get_next_token(&lexer);
    Token number = get_next_token(&lexer);
    Token next = get_next_token(&lexer);
    Token arrow = get_next_token(&lexer);
    Token result = get_next_token(&lexer);
    Token end = get_next_token(&lexer);
    assert(name.source_path == path && name.line_number == 7 && name.column_number == 3);
    assert(number.column_number == 6 && number.line_number == 7);
    assert(next.column_number == 2 && next.line_number == 9);
    assert(arrow.column_number == 7 && result.column_number == 9);
    assert(end.type == TOKEN_END_OF_FILE && end.line_number == 10 && end.column_number == 6);
    destroy_test();
}

static void it_distinguishes_call_markers_from_loop_exits_and_tracks_adjacency(void) {
    setup_test();
    set_source("\u5546\u54c1!\r\n ^\n");
    Token target = get_next_token(&lexer);
    Token marker = get_next_token(&lexer);
    Token exit = get_next_token(&lexer);
    assert(target.type == TOKEN_IDENTIFIER && marker.type == TOKEN_CALL_MARKER);
    assert(marker.line_number == target.line_number);
    assert(marker.column_number == target.column_number + strlen(target.lexeme));
    assert(!marker.is_at_line_start && exit.type == TOKEN_BREAK && exit.column_number == 2);
    destroy_test();
}

static void it_recognizes_a_contiguous_double_arrow_without_merging_separated_arrows(void) {
    setup_test();
    set_source(">> > >\r\n>\n>>");
    Token retained = get_next_token(&lexer);
    Token first = get_next_token(&lexer);
    Token second = get_next_token(&lexer);
    Token returned = get_next_token(&lexer);
    Token last = get_next_token(&lexer);
    Token end = get_next_token(&lexer);
    assert(retained.type == TOKEN_RETAINED_RESULT_ARROW && strcmp(retained.lexeme, ">>") == 0);
    assert(first.type == TOKEN_RESULT_ARROW && first.column_number == 4);
    assert(second.type == TOKEN_RESULT_ARROW && second.column_number == 6);
    assert(returned.type == TOKEN_RESULT_ARROW && returned.line_number == 2);
    assert(last.type == TOKEN_RETAINED_RESULT_ARROW && last.line_number == 3 && last.column_number == 1);
    assert(end.type == TOKEN_END_OF_FILE && end.column_number == 3);
    destroy_test();
}

static void it_preserves_errors_from_arrow_lookahead(void) {
    setup_test();
    set_source(">\rbroken");
    Token arrow = get_next_token(&lexer);
    assert(arrow.type == TOKEN_ERROR && error.error_code == ERROR_INVALID);
    destroy_test();
}

void run(void) {
    it_recognizes_a_contiguous_double_arrow_without_merging_separated_arrows();
    it_preserves_errors_from_arrow_lookahead();
    it_distinguishes_call_markers_from_loop_exits_and_tracks_adjacency();
    it_tracks_byte_columns_across_unicode_comments_and_crlf();
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
