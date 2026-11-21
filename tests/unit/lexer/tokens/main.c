/**
 * @file
 * @brief Verify exact lexical boundaries without declarations or runtime name lookup.
 */
#include <stdio.h>
#include "../common.c"
#include "lexer/tokens/tokens.h"

/** Read one fixture through real source/layout ownership and initialize its exact structural region. */
static bool begin(const char *text, size_t indentation, LexCursor *cursor) {
    LayoutStatus status = read_layout(text);
    assert(status == LAYOUT_OK);
    return lex_line_begin(cursor, layout.first, indentation);
}

/** Every valid join retains complete names and longest punctuation across physical ending variants. */
static void it_preserves_complete_names_compound_tokens_and_all_line_ending_forms(void) {
    const char *texts[] = {"//商店 工具/商品 状态", "目标&[] !", "targets&[positions[next position]]",
        "input name& < & item state", "% target&", "word name   # arbitrary   comment"};
    const LexKind kinds[][8] = {
        {LEX_MODULE_PATH}, {LEX_NAME,LEX_REFERENCE_LIST,LEX_EXECUTE},
        {LEX_NAME,LEX_REFERENCE_INDEX,LEX_NAME,LEX_INDEX_OPEN,LEX_NAME,LEX_INDEX_CLOSE,LEX_INDEX_CLOSE},
        {LEX_NAME,LEX_REFERENCE,LEX_INPUT,LEX_REFERENCE,LEX_NAME},
        {LEX_CONTINUATION,LEX_NAME,LEX_REFERENCE}, {LEX_NAME}
    };
    const size_t counts[] = {1,3,7,5,3,1};
    const char *endings[] = {"", "\n", "\r\n"};
    for (size_t example = 0; example < 6; example++) {
        for (size_t ending = 0; ending < 3; ending++) {
            char text[256];
            int written = snprintf(text, sizeof(text), "%s%s", texts[example], endings[ending]);
            assert(written > 0 && (size_t)written < sizeof(text));
            setup_test();
            LexCursor cursor;
            bool started = begin(text, 0, &cursor);
            assert(started);
            size_t allocations = allocation_count;
            failure_at = allocation_count + 1;
            LexToken token;
            for (size_t index = 0; index < counts[example]; index++) {
                LexStatus status = lex_token_next(&cursor, &token);
                assert(status == LEX_TOKEN && token.kind == kinds[example][index]);
                assert(source_span_valid(token.text));
                if (example == 0 && index == 0) { assert(layout_text_is(token.text, "//商店 工具/商品 状态")); }
                if (example == 2 && index == 4) { assert(layout_text_is(token.text, "next position")); }
            }
            LexStatus status = lex_token_next(&cursor, &token);
            assert(status == LEX_END && allocation_count == allocations);
            destroy_layout();
        }
    }
}

/** A malformed join fails at the lexical boundary and never publishes a partial token. */
static void it_rejects_spacing_near_compact_accesses_and_separated_operators(void) {
    const char *invalid[] = {"target!", "target%", "%target", "target &", "&target", "&  target", "values []",
        "values &[]", "values[ position]", "values[position ]", ". shop", ".. shop", ".shop .item",
        "..shop..item", "input< value", "input <value", ">>value", "name  part", "name  !",
        "input name  < |#data", "input name <  |#data", "input name< |#data",
        "@ !", "values& []", "values& [position]", "value > > result", "x\xc2\xa0y", "2name", "x 2name", "x+"};
    for (size_t example = 0; example < sizeof(invalid) / sizeof(invalid[0]); example++) {
        setup_test();
        LexCursor cursor;
        bool started = begin(invalid[example], 0, &cursor);
        assert(started);
        LexToken token = {.kind = LEX_BREAK};
        LexStatus status;
        do {
            unsigned char previous[sizeof(token)];
            memcpy(previous, &token, sizeof(token));
            status = lex_token_next(&cursor, &token);
            if (status == LEX_INVALID) { assert(memcmp(previous, &token, sizeof(token)) == 0); }
        } while (status == LEX_TOKEN);
        assert(status == LEX_INVALID && diagnostic_valid(&cursor.error));
        assert(cursor.error.expected != NULL);
        destroy_layout();
    }
}

/** Exact physical indentation applies to comments; empty lines ignore depth while spaces-only/trailing code lines fail. */
static void it_checks_region_comment_indentation_and_distinguishes_comment_prefix_spaces(void) {
    const char *valid[] = {"  # text   # more", "\n", "  name     # comment", "  | raw   "};
    for (size_t index = 0; index < 4; index++) {
        setup_test();
        LexCursor cursor;
        bool started = begin(valid[index], 2, &cursor);
        assert(started);
        destroy_layout();
    }
    const char *invalid[] = {" # shallow", "   # too deep", "  ", "  name "};
    for (size_t index = 0; index < 4; index++) {
        setup_test();
        LexCursor cursor;
        bool started = begin(invalid[index], 2, &cursor);
        assert(!started && diagnostic_valid(&cursor.error));
        destroy_layout();
    }
}

/** Named and positional entry literals recognize complete labels before # and preserve every permitted payload byte. */
static void it_preserves_literal_payloads_after_exact_multiword_labels_and_storage_suffixes(void) {
    const char *labels[] = {"<", "input name <", "input name& <", "input name[] <", "input name&[] <"};
    for (size_t index = 0; index < 5; index++) {
        setup_test();
        char text[128];
        int written = snprintf(text, sizeof(text), " %s |  raw #  |@!  \r\n", labels[index]);
        assert(written > 0 && (size_t)written < sizeof(text));
        LexCursor cursor;
        bool started = begin(text, 1, &cursor);
        assert(started && layout.first->kind == LINE_INPUT_LITERAL);
        LexToken token;
        LexStatus status;
        do {
            status = lex_token_next(&cursor, &token);
            if (status == LEX_TOKEN && token.kind == LEX_LITERAL) {
                assert(layout_text_is(token.payload, "  raw #  |@!  "));
            }
        } while (status == LEX_TOKEN);
        assert(status == LEX_END && token.kind == LEX_LITERAL);
        destroy_layout();
    }
}

/** TAB and lone CR are rejected before sections, comments, literals or drafts can hide them. */
static void it_rejects_global_tabs_and_lone_cr_before_any_literal_or_activation_interpretation(void) {
    const char *contexts[] = {"draft\t", "# comment\t", " |literal\t", " < |literal\t", "@!\n\tcode", "draft\r"};
    for (size_t index = 0; index < 6; index++) {
        setup_test();
        LayoutStatus status = read_layout(contexts[index]);
        assert(status == LAYOUT_INVALID && diagnostic_valid(&layout.diagnostic));
        assert(layout.first == NULL);
        destroy_layout();
    }
}

/** Fixed annotations, empty literals, self reference and standalone markers retain their exact lexical categories. */
static void it_recognizes_every_annotation_and_standalone_marker(void) {
    const char *spellings[] = {"@=", "@<", "@>", "@!", "@C", "@...", "&", "%", "<", ">", "?", ":", "{", "}", "^", "|"};
    const LexKind expected[] = {LEX_CONSTANTS,LEX_PARAMETERS,LEX_RESULTS,LEX_AMA,LEX_NATIVE,LEX_STATIC_MODULE,
        LEX_REFERENCE,LEX_CONTINUATION,LEX_INPUT,LEX_OUTPUT,LEX_QUESTION,LEX_COLON,LEX_LOOP_OPEN,
        LEX_LOOP_CLOSE,LEX_BREAK,LEX_LITERAL};
    for (size_t index = 0; index < sizeof(expected) / sizeof(expected[0]); index++) {
        setup_test();
        LexCursor cursor;
        bool started = begin(spellings[index], 0, &cursor);
        assert(started);
        LexToken token;
        LexStatus status = lex_token_next(&cursor, &token);
        assert(status == LEX_TOKEN && token.kind == expected[index]);
        status = lex_token_next(&cursor, &token);
        assert(status == LEX_END);
        destroy_layout();
    }
    setup_test();
    LexCursor cursor;
    bool started = begin("name >> result", 0, &cursor);
    assert(started);
    const LexKind read[] = {LEX_NAME,LEX_READ,LEX_NAME};
    for (size_t index = 0; index < 3; index++) {
        LexToken token;
        LexStatus status = lex_token_next(&cursor, &token);
        assert(status == LEX_TOKEN && token.kind == read[index]);
    }
    destroy_layout();
}

/** The shared name scanner respects bounds and preserves outputs when no valid starting word exists. */
static void it_shares_bounded_maximal_name_scanning_with_parser_consumers(void) {
    setup_test();
    LayoutStatus read = read_layout("商品 状态[]");
    assert(read == LAYOUT_OK);
    SourceSpan text = layout.first->code;
    SourceSpan name = {0};
    size_t cursor = text.start;
    bool found = lex_name(text, &cursor, &name);
    assert(found && layout_text_is(name, "商品 状态"));
    assert(cursor == strlen("商品 状态"));
    SourceSpan previous = name;
    size_t previous_cursor = cursor;
    found = lex_name(text, &cursor, &name);
    assert(!found && cursor == previous_cursor);
    assert(name.source == previous.source && name.start == previous.start && name.end == previous.end);
    text.start = text.end;
    found = lex_name(text, &cursor, &name);
    assert(!found && cursor == previous_cursor);
    destroy_layout();
}

/** Compose shared hyphen words with path roots, compact storage/index syntax
 * and raw entry literals; malformed joins must fail rather than split names.
 */
static void it_uses_hyphen_words_in_paths_accesses_and_literal_labels(void) {
    const char *valid[] = {"//shop-tools/item-2", "result-name&[position-2]",
        "name-2 part_3 !"};
    const LexKind kinds[][4] = {
        {LEX_MODULE_PATH},
        {LEX_NAME, LEX_REFERENCE_INDEX, LEX_NAME, LEX_INDEX_CLOSE},
        {LEX_NAME, LEX_EXECUTE}
    };
    const size_t counts[] = {1, 4, 2};
    for (size_t index = 0; index < 3; index++) {
        setup_test();
        LexCursor cursor;
        assert(begin(valid[index], 0, &cursor));
        LexToken token;
        for (size_t position = 0; position < counts[index]; position++) {
            assert(lex_token_next(&cursor, &token) == LEX_TOKEN);
            assert(token.kind == kinds[index][position]);
            if (index == 0 && position == 0) {
                assert(layout_text_is(token.text, "//shop-tools/item-2"));
            }
            if (index == 2 && position == 0) {
                assert(layout_text_is(token.text, "name-2 part_3"));
            }
        }
        assert(lex_token_next(&cursor, &token) == LEX_END);
        destroy_layout();
    }
    setup_test();
    LexCursor cursor;
    assert(begin(" input-name < |-- raw # -  \n", 1, &cursor));
    LexToken token;
    assert(lex_token_next(&cursor, &token) == LEX_TOKEN && token.kind == LEX_NAME);
    assert(layout_text_is(token.text, "input-name"));
    assert(lex_token_next(&cursor, &token) == LEX_TOKEN && token.kind == LEX_INPUT);
    assert(lex_token_next(&cursor, &token) == LEX_TOKEN && token.kind == LEX_LITERAL);
    assert(layout_text_is(token.payload, "-- raw # -  "));
    assert(lex_token_next(&cursor, &token) == LEX_END);
    destroy_layout();
    const char *invalid[] = {"a-", "-a", "a--b", "a- b", "a -b",
        ".shop-.item", "..shop..item-2"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_test();
        assert(begin(invalid[index], 0, &cursor));
        LexStatus status;
        do { status = lex_token_next(&cursor, &token); } while (status == LEX_TOKEN);
        assert(status == LEX_INVALID && diagnostic_valid(&cursor.error));
        destroy_layout();
    }
}

/** One dot adjoins complete static names; no positional prefix or padded join is accepted.
 */
static void it_distinguishes_static_prefixes_from_module_roots_and_binding_separators(void) {
    const char *valid[] = {".key-type.part < //integer", ".input < .value",
        ".output > .received-type"};
    const LexKind kinds[][7] = {
        {LEX_DOT, LEX_NAME, LEX_DOT, LEX_NAME, LEX_INPUT, LEX_MODULE_PATH},
        {LEX_DOT, LEX_NAME, LEX_INPUT, LEX_DOT, LEX_NAME},
        {LEX_DOT, LEX_NAME, LEX_OUTPUT, LEX_DOT, LEX_NAME}
    };
    const size_t checked[] = {6, 5, 5};
    for (size_t index = 0; index < 3; index++) {
        setup_test();
        LexCursor cursor;
        assert(begin(valid[index], 0, &cursor));
        LexToken token;
        for (size_t ordinal = 0; ordinal < checked[index]; ordinal++) {
            assert(lex_token_next(&cursor, &token) == LEX_TOKEN);
            assert(token.kind == kinds[index][ordinal]);
        }
        assert(lex_token_next(&cursor, &token) == LEX_END);
        destroy_layout();
    }
    const char *invalid[] = {"....type", "... type", "...< ..integer",
        "...type..part", "...type...part", "... <", "... >", "... > ...received-type"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_test();
        LexCursor cursor;
        assert(begin(invalid[index], 0, &cursor));
        LexToken token;
        LexStatus status;
        do { status = lex_token_next(&cursor, &token); } while (status == LEX_TOKEN);
        assert(status == LEX_INVALID);
        destroy_layout();
    }
}

/** Paths remain single operands even with version dots, multiword names and
 * compact constant-list suffixes; static names never become filesystem paths.
 */
static void it_scans_path_operands_and_rejects_invalid_component_joins(void) {
    const char *paths[] = {"/acme/pkg/v2.4.14-speed/src/item", "//item name/member",
        "./item", "../../item", "~containers/item name", "~local"};
    for (size_t index = 0; index < sizeof(paths) / sizeof(*paths); index++) {
        setup_test();
        LexCursor cursor;
        assert(begin(paths[index], 0, &cursor));
        LexToken token;
        assert(lex_token_next(&cursor, &token) == LEX_TOKEN);
        assert(token.kind == LEX_MODULE_PATH && layout_text_is(token.text, paths[index]));
        assert(lex_token_next(&cursor, &token) == LEX_END);
        destroy_layout();
    }
    setup_test();
    LexCursor cursor;
    assert(begin("//table/items[]", 0, &cursor));
    LexToken token;
    assert(lex_token_next(&cursor, &token) == LEX_TOKEN && token.kind == LEX_MODULE_PATH);
    assert(lex_token_next(&cursor, &token) == LEX_TOKEN && token.kind == LEX_LIST);
    assert(lex_token_next(&cursor, &token) == LEX_END);
    destroy_layout();
    const char *invalid[] = {"/", "//", "///item", "//item/", "//item//next",
        "./", "../", "~", "~ item", "/a/v01.2.3/src/item", "/a/v1.2.3.4/src/item",
        "//item.child", "//item []", "name//item", ".type/field", ".type..field"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        setup_test();
        assert(begin(invalid[index], 0, &cursor));
        LexStatus status;
        do { status = lex_token_next(&cursor, &token); } while (status == LEX_TOKEN);
        assert(status == LEX_INVALID && diagnostic_valid(&cursor.error));
        destroy_layout();
    }
}

/** Run every exact-layout/token boundary scenario explicitly. */
void run(void) {
    it_scans_path_operands_and_rejects_invalid_component_joins();
    it_distinguishes_static_prefixes_from_module_roots_and_binding_separators();
    it_uses_hyphen_words_in_paths_accesses_and_literal_labels();
    it_preserves_complete_names_compound_tokens_and_all_line_ending_forms();
    it_recognizes_every_annotation_and_standalone_marker();
    it_shares_bounded_maximal_name_scanning_with_parser_consumers();
    it_rejects_spacing_near_compact_accesses_and_separated_operators();
    it_checks_region_comment_indentation_and_distinguishes_comment_prefix_spaces();
    it_preserves_literal_payloads_after_exact_multiword_labels_and_storage_suffixes();
    it_rejects_global_tabs_and_lone_cr_before_any_literal_or_activation_interpretation();
}
/** Return success only after all assertions finish. */
int main(void) { run(); return EXIT_SUCCESS; }
