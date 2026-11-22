/** @file
 * @brief Verify strict response JSON with independent exact models and deep iterative parsing.
 * @see [T224 contract](../../../../../docs/implementation/source-operations.md)
 */
#include "../../common.c"
#include "source/json/json.h"
static SourceArena parsed;
static const SourceText *input;
static const SourceJsonValue *root;
static SourceJsonIssue issue;
/** Count every source/tree allocation and retain exact response bytes for spans. */
static void prepare(const void *bytes, size_t length) {
    setup_test(); source_arena_init(&parsed, sources.allocator);
    SourceStatus added = source_manager_add(&sources, "response.json", bytes, length, &input);
    assert(added == SOURCE_OK); root = NULL; issue = (SourceJsonIssue){0};
}
/** Retire decoded trees before their borrowed original response. */
static void finish(void) { source_arena_destroy(&parsed); destroy_test(); }
/** Compare a decoded or borrowed length-delimited value, including embedded NUL. */
static void equals(SourceJsonText text, const void *bytes, size_t length) {
    assert(text.length == length && !memcmp(text.bytes, bytes, length));
    assert(text.origin.source == input && source_span_valid(text.origin));
}
/** Select one known test key without first/last-match or C-string assumptions. */
static const SourceJsonValue *member(const SourceJsonValue *object, const char *name) {
    assert(object->kind == SOURCE_JSON_OBJECT);
    for (const SourceJsonValue *value = object->first; value; value = value->next) {
        if (value->key.length == strlen(name) && !memcmp(value->key.bytes, name, value->key.length)) { return value; }
    }
    assert(false); return NULL;
}
/** Objects/arrays retain source order, scoped equal names and complete spans;
 * whitespace follows JSON rather than Ama indentation rules. */
static void it_preserves_nested_structure_keys_scalars_and_original_spans(void) {
    const char *text = " \t\r\n{\"a\":[true,false,null,{\"a\":\"Имя\"}],\"\":{}}\n";
    prepare(text, strlen(text));
    SourceJsonStatus status = source_json_parse(&parsed, input, &root, &issue);
    assert(status == SOURCE_JSON_OK && root->kind == SOURCE_JSON_OBJECT && !root->next);
    assert(root->origin.start == 4 && root->origin.end == strlen(text) - 1);
    const SourceJsonValue *array = member(root, "a");
    assert(array->kind == SOURCE_JSON_ARRAY && array->first->kind == SOURCE_JSON_BOOLEAN && array->first->boolean);
    const SourceJsonValue *value = array->first->next;
    assert(value->kind == SOURCE_JSON_BOOLEAN && !value->boolean);
    value = value->next; assert(value->kind == SOURCE_JSON_NULL);
    value = value->next; assert(value->kind == SOURCE_JSON_OBJECT && !value->next);
    equals(member(value, "a")->text, "Имя", strlen("Имя"));
    assert(member(root, "")->kind == SOURCE_JSON_OBJECT && !member(root, "")->first);
    assert(!issue.message);
    finish();
}
/** Explicit codepoint boundary bytes independently check all UTF-8 widths,
 * surrogate pairing, controls, escaped NUL, quotes, slash and backslash. */
static void it_decodes_unicode_boundaries_controls_and_embedded_nul_exactly(void) {
    const char *text = "\"\\u0000\\u007f\\u0080\\u07ff\\u0800\\ud7ff\\ue000\\uffff\\ud800\\udc00\\udbff\\udfff\\b\\f\\n\\r\\t\\\"\\\\\\/\"";
    const unsigned char expected[] = {0, 0x7f, 0xc2, 0x80, 0xdf, 0xbf, 0xe0, 0xa0, 0x80,
        0xed, 0x9f, 0xbf, 0xee, 0x80, 0x80, 0xef, 0xbf, 0xbf,
        0xf0, 0x90, 0x80, 0x80, 0xf4, 0x8f, 0xbf, 0xbf, '\b', '\f', '\n', '\r', '\t', '"', '\\', '/'};
    prepare(text, strlen(text));
    SourceJsonStatus status = source_json_parse(&parsed, input, &root, &issue);
    assert(status == SOURCE_JSON_OK && root->kind == SOURCE_JSON_STRING);
    equals(root->text, expected, sizeof(expected));
    assert(root->text.bytes[sizeof(expected)] == 0);
    finish();
}
/** Arbitrarily wide decimal spellings are retained without rounding, overflow,
 * locale conversion or an invented integer/exponent bound. */
static void it_retains_exact_json_number_spellings_without_conversion(void) {
    const char *numbers[] = {"0", "-0", "1", "-1", "10", "0.1", "-0.1", "1e0", "1E+1", "1e-1",
        "1844674407370955161518446744073709551615e+999999999999999999999999"};
    for (size_t index = 0; index < sizeof(numbers) / sizeof(*numbers); index++) {
        prepare(numbers[index], strlen(numbers[index]));
        SourceJsonStatus status = source_json_parse(&parsed, input, &root, &issue);
        assert(status == SOURCE_JSON_OK && root->kind == SOURCE_JSON_NUMBER);
        equals(root->text, numbers[index], strlen(numbers[index]));
        assert(root->text.bytes == source_bytes(input));
        finish();
    }
}
/** Malformed delimiters, escapes, duplicate decoded keys, non-JSON values and
 * trailing material cannot publish a partial or guessed response. */
static void it_refuses_malformed_documents_and_duplicate_decoded_keys(void) {
    const char *invalid[] = {"", " ", "[", "{", "[1,]", "{\"a\":1,}", "{\"a\" 1}", "{\"a\":}", "[1 2]",
        "true false", "nullx", "NaN", "Infinity", "+1", "01", "-01", "-", "1.", "1e", "1e+", "00", "1e-",
        "\"\n\"", "\"\\q\"", "\"\\u12\"", "\"\\uGGGG\"", "\"\\ud800\"", "\"\\ud800\\u0001\"",
        "\"\\udc00\"", "\"\\ud800\\n\"", "{\"a\":1,\"\\u0061\":2}", "{\"\\u0000\":1,\"\\u0000\":2}",
        "{true:1}", "['x']", "{]", "[}", "\"trailing\\", "tru", "fals", "nul", "falsex"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        prepare(invalid[index], strlen(invalid[index]));
        SourceJsonValue sentinel = {0}; root = &sentinel;
        SourceJsonStatus status = source_json_parse(&parsed, input, &root, &issue);
        assert(status == SOURCE_JSON_INVALID && root == &sentinel && issue.message);
        assert(issue.origin.source == input && source_span_valid(issue.origin));
        finish();
    }
}
/** UTF-8 errors are refused at their original byte without replacement. */
static void it_refuses_malformed_utf8_before_syntax_interpretation(void) {
    const unsigned char invalid[][4] = {{0xc0, 0x80}, {0x80}, {0xed, 0xa0, 0x80},
        {0xf4, 0x90, 0x80, 0x80}, {0xf0, 0x90}};
    const size_t sizes[] = {2, 1, 3, 4, 2};
    for (size_t index = 0; index < sizeof(sizes) / sizeof(*sizes); index++) {
        prepare(invalid[index], sizes[index]);
        SourceJsonValue sentinel = {0}; root = &sentinel;
        SourceJsonStatus status = source_json_parse(&parsed, input, &root, &issue);
        assert(status == SOURCE_JSON_INVALID && root == &sentinel);
        assert(!strcmp(issue.message, "JSON response is not valid UTF-8") && issue.origin.start == 0);
        finish();
    }
}
/** A rejected byte or duplicate key is located at its own original occurrence,
 * rather than the later closing quote/value after it was consumed. */
static void it_locates_rejected_controls_escapes_and_duplicate_keys_exactly(void) {
    const char *texts[] = {"{\"a\":1,\"\\u0061\":2}", "\"a\nb\"", "\"\\q\"", "\"\\uDC00\"", "\"\\uD800\\u0001\""};
    const size_t positions[] = {7, 2, 2, 3, 9};
    for (size_t index = 0; index < sizeof(positions) / sizeof(*positions); index++) {
        prepare(texts[index], strlen(texts[index]));
        SourceJsonStatus status = source_json_parse(&parsed, input, &root, &issue);
        assert(status == SOURCE_JSON_INVALID && issue.origin.start == positions[index]);
        assert(issue.origin.end == positions[index] + 1);
        finish();
    }
}
/** Deep well-formed input consumes explicit heap continuations instead of
 * recursively exhausting the C call stack or hitting an arbitrary cutoff. */
static void it_parses_twenty_thousand_nested_containers_without_recursion(void) {
    size_t depth = 20000, length = 2 * depth + 4;
    unsigned char *bytes = malloc(length); assert(bytes);
    memset(bytes, '[', depth); memcpy(bytes + depth, "null", 4); memset(bytes + depth + 4, ']', depth);
    prepare(bytes, length); free(bytes);
    SourceJsonStatus status = source_json_parse(&parsed, input, &root, &issue);
    assert(status == SOURCE_JSON_OK && root->origin.end == length);
    const SourceJsonValue *value = root;
    for (size_t index = 0; index < depth; index++) {
        assert(value->kind == SOURCE_JSON_ARRAY && value->first && !value->first->next);
        value = value->first;
    }
    assert(value->kind == SOURCE_JSON_NULL);
    finish();
}
/** Every value/frame/key/string allocation refusal preserves output and
 * reclaims all private partial work at arena destruction. */
static void it_preserves_output_on_every_json_allocation_failure(void) {
    const char *text = "{\"a\":[\"\\ud800\\udc00\",1,true,null],\"b\":{\"c\":\"value\"}}";
    size_t allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        prepare(text, strlen(text)); SourceJsonValue sentinel = {0}; root = &sentinel;
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceJsonStatus status = source_json_parse(&parsed, input, &root, &issue);
        if (!failure) { assert(status == SOURCE_JSON_OK); allocations = allocation_count - before; assert(allocations); }
        else {
            assert(status == SOURCE_JSON_OUT_OF_MEMORY && root == &sentinel && issue.message);
            assert(issue.origin.source == input && source_span_valid(issue.origin));
        }
        failure_at = 0; finish();
    }
}
/** Invalid owners are refused without publishing caller output or dereferencing
 * a missing allocator/source/diagnostic. */
static void it_refuses_invalid_json_parser_arguments_without_publication(void) {
    prepare("null", 4); SourceJsonValue sentinel = {0}; root = &sentinel;
    assert(source_json_parse(NULL, input, &root, &issue) == SOURCE_JSON_INVALID);
    assert(source_json_parse(&parsed, NULL, &root, &issue) == SOURCE_JSON_INVALID);
    assert(source_json_parse(&parsed, input, NULL, &issue) == SOURCE_JSON_INVALID);
    assert(source_json_parse(&parsed, input, &root, NULL) == SOURCE_JSON_INVALID);
    assert(root == &sentinel); finish();
}
/** Execute structure, lexical, deep-input, source and allocation contracts. */
void run(void) {
    it_preserves_nested_structure_keys_scalars_and_original_spans();
    it_decodes_unicode_boundaries_controls_and_embedded_nul_exactly();
    it_retains_exact_json_number_spellings_without_conversion();
    it_refuses_malformed_documents_and_duplicate_decoded_keys();
    it_refuses_malformed_utf8_before_syntax_interpretation();
    it_locates_rejected_controls_escapes_and_duplicate_keys_exactly();
    it_parses_twenty_thousand_nested_containers_without_recursion();
    it_preserves_output_on_every_json_allocation_failure();
    it_refuses_invalid_json_parser_arguments_without_publication();
}
/** Keep all assertions active in this independently runnable leaf. */
int main(void) { run(); return 0; }
