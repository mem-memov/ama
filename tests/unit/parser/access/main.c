/**
 * @file
 * @brief Check structural value/reference/list access and source forms before semantic resolution.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../../source/common.c"
#include "parser/internal.h"

static AstModule module;
static AstAccess *access;

/**
 * Parse one explicit structural access/source fixture and retain source-backed outputs until
 * fixture destruction.
 */
static bool parse_access(const char *text) {
    setup_test();
    ast_module_init(&module, sources.allocator);
    const SourceText *source;
    SourceStatus added = source_manager_add(&sources, "access", (const unsigned char *)text, strlen(text), &source);
    assert(added == SOURCE_OK);
    Parser parser = {&module, NULL, NULL, false};
    access = NULL;
    return parser_access(&parser, (SourceSpan){source, 0, strlen(text)}, &access);
}

/**
 * Destroy access/AST scratch before its source storage and counted allocator owner.
 */
static void destroy_access(void) {
    ast_module_destroy(&module);
    destroy_test();
}

/**
 * Construct the explicitly bounded structural access shape used to compare parser output and
 * allocation failure paths.
 */
static bool model_access(const char *text, size_t length, bool root) {
    size_t name = 0;
    while (name < length && text[name] == 'a') {
        name++;
    }
    if (name == 0) {
        return false;
    }
    if (root && name < length && text[name] == '&') {
        name++;
    }
    if (name == length) {
        return true;
    }
    if (text[name] != '[' || text[length - 1] != ']') {
        return false;
    }
    size_t inner = length - name - 2;
    return inner == 0 ? root : model_access(text + name + 1, inner, false);
}

/**
 * Assert that the component matches the access grammar for all short suffix strings. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_matches_the_access_grammar_for_all_short_suffix_strings(void) {
    const char alphabet[] = "a&[]";
    size_t combinations = 1;
    for (size_t length = 0; length <= 7; length++, combinations *= 4) {
        for (size_t number = 0; number < combinations; number++) {
            char text[8] = {0};
            size_t remaining = number;
            for (size_t index = 0; index < length; index++) {
                text[index] = alphabet[remaining % 4];
                remaining /= 4;
            }
            bool expected = model_access(text, length, true);
            bool accepted = parse_access(text);
            assert(accepted == expected);
            assert(accepted ? access != NULL : access == NULL && diagnostic_valid(&module.error.diagnostic));
            destroy_access();
        }
    }
}

/**
 * Assert that the component preserves exact unicode and nested access spans. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_preserves_exact_unicode_and_nested_access_spans(void) {
    bool accepted = parse_access("商品&[位置[索引]]");
    assert(accepted && access->reference && !access->whole_list);
    assert(layout_text_is(access->name, "商品"));
    assert(layout_text_is(access->index->span, "位置[索引]"));
    assert(layout_text_is(access->index->index->span, "索引"));
    destroy_access();
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_matches_the_access_grammar_for_all_short_suffix_strings();
    it_preserves_exact_unicode_and_nested_access_spans();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
