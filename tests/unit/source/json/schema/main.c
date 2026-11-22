/** @file
 * @brief Verify exact protocol field sets, redundant byte representations and bounded integer strings.
 * @see [T224 contract](../../../../../docs/implementation/source-operations.md)
 */
#include <stdint.h>
#include <stdio.h>
#include "../../common.c"
#include "source/json/schema/schema.h"
static SourceArena decoded;
static const SourceText *input;
static const SourceJsonValue *root;
static SourceJsonIssue issue;
/** Schema tests consume real syntax trees, retaining all counted owners. */
static void prepare(const char *text) {
    setup_test(); source_arena_init(&decoded, sources.allocator);
    SourceStatus added = source_manager_add(&sources, "response.json", (const unsigned char *)text, strlen(text), &input);
    assert(added == SOURCE_OK); root = NULL; issue = (SourceJsonIssue){0};
    SourceJsonStatus status = source_json_parse(&decoded, input, &root, &issue);
    assert(status == SOURCE_JSON_OK);
}
/** Retire decoded payload/tree storage before original response sources. */
static void finish(void) { source_arena_destroy(&decoded); destroy_test(); }
/** Field names are exact decoded bytes and never impose response ordering. */
static void it_matches_exact_unordered_fields_without_prefixes_or_partial_publication(void) {
    const char *const names[] = {"a", "b"};
    prepare("{\"b\":2,\"a\":1}");
    const SourceJsonValue *fields[2] = {NULL, NULL};
    SourceJsonStatus status = source_json_schema_fields(root, names, 2, fields, &issue);
    assert(status == SOURCE_JSON_OK && fields[0]->text.bytes[0] == '1' && fields[1]->text.bytes[0] == '2');
    finish();
    const char *invalid[] = {"{\"a\":1}", "{\"a\":1,\"b\":2,\"extra\":3}", "{\"a\\u0000\":1,\"b\":2}", "[]", "null"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        prepare(invalid[index]); fields[0] = fields[1] = root;
        status = source_json_schema_fields(root, names, 2, fields, &issue);
        assert(status == SOURCE_JSON_INVALID && fields[0] == root && fields[1] == root && issue.message);
        assert(issue.origin.source == input); finish();
    }
    prepare("{}"); status = source_json_schema_fields(root, NULL, 0, NULL, &issue);
    assert(status == SOURCE_JSON_OK); finish();
}
/** Every byte, including NUL and malformed UTF-8, survives exact hex decoding
 * with no replacement. Independently known Unicode text checks redundancy. */
static void it_decodes_all_byte_values_and_consistent_unicode_views_exactly(void) {
    char text[600]; const char *digits = "0123456789abcdef";
    size_t count = (size_t)snprintf(text, sizeof(text), "{\"utf8\":null,\"hex\":\"");
    for (unsigned value = 0; value < 256; value++) {
        text[count++] = digits[value >> 4]; text[count++] = digits[value & 15];
    }
    memcpy(text + count, "\"}", 3);
    prepare(text); SourceJsonText bytes = {0};
    SourceJsonStatus status = source_json_schema_bytes(&decoded, root, &bytes, &issue);
    assert(status == SOURCE_JSON_OK && bytes.length == 256 && bytes.bytes[256] == 0);
    for (unsigned value = 0; value < 256; value++) { assert(bytes.bytes[value] == value); }
    finish();
    prepare("{\"hex\":\"d098d0bcd18f\",\"utf8\":\"\\u0418\\u043c\\u044f\"}");
    status = source_json_schema_bytes(&decoded, root, &bytes, &issue);
    assert(status == SOURCE_JSON_OK && bytes.length == strlen("Имя") && !memcmp(bytes.bytes, "Имя", bytes.length));
    finish();
    prepare("{\"hex\":\"\",\"utf8\":\"\"}");
    status = source_json_schema_bytes(&decoded, root, &bytes, &issue);
    assert(status == SOURCE_JSON_OK && !bytes.length && !bytes.bytes[0]); finish();
}
/** Hex must be lowercase/even and both representations must agree exactly. */
static void it_refuses_noncanonical_or_inconsistent_byte_objects_without_publication(void) {
    const char *invalid[] = {"{\"utf8\":null,\"hex\":\"\"}", "{\"utf8\":\"\",\"hex\":\"ff\"}",
        "{\"utf8\":\"a\",\"hex\":\"62\"}", "{\"utf8\":true,\"hex\":\"ff\"}", "{\"utf8\":null,\"hex\":\"FF\"}",
        "{\"utf8\":null,\"hex\":\"fg\"}", "{\"utf8\":null,\"hex\":\"f\"}", "{\"utf8\":\"\",\"hex\":0}",
        "{\"utf8\":\"\"}", "{\"utf8\":\"\",\"hex\":\"\",\"extra\":null}"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        prepare(invalid[index]);
        SourceJsonText bytes = {(const unsigned char *)"sentinel", 8, {input, 0, 0}};
        SourceJsonStatus status = source_json_schema_bytes(&decoded, root, &bytes, &issue);
        assert(status == SOURCE_JSON_INVALID && bytes.length == 8 && !memcmp(bytes.bytes, "sentinel", 8));
        assert(bytes.origin.source == input && bytes.origin.start == 0 && bytes.origin.end == 0);
        assert(issue.message && issue.origin.source == input); finish();
    }
}
/** Size conversion accepts the full host range and rejects every grammar and
 * overflow partition before modifying caller output. */
static void it_checks_canonical_decimal_strings_at_size_boundaries(void) {
    char maximum[3 * sizeof(size_t) + 8];
    int length = snprintf(maximum, sizeof(maximum), "\"%zu\"", SIZE_MAX);
    assert(length > 0 && (size_t)length < sizeof(maximum));
    prepare(maximum); size_t value = 0;
    SourceJsonStatus status = source_json_schema_size(root, &value, &issue);
    assert(status == SOURCE_JSON_OK && value == SIZE_MAX); finish();
    prepare("\"0\""); value = 99; status = source_json_schema_size(root, &value, &issue);
    assert(status == SOURCE_JSON_OK && value == 0); finish();
    maximum[length - 1] = '0'; maximum[length] = '"'; maximum[length + 1] = 0;
    const char *invalid[] = {maximum, "\"\"", "\"01\"", "\"-1\"", "\"+1\"", "\" 1\"", "\"1 \"", "\"1e2\"", "\"0.0\"", "0", "true", "null"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        prepare(invalid[index]); value = 99;
        status = source_json_schema_size(root, &value, &issue);
        assert(status == SOURCE_JSON_INVALID && value == 99 && issue.origin.source == input); finish();
    }
}
/** Filesystem identities retain the full unsigned host range independently of
 * size_t; overflow and non-string numeric values remain explicit failures. */
static void it_retains_full_width_unsigned_identity_strings(void) {
    char maximum[3 * sizeof(uintmax_t) + 8];
    int length = snprintf(maximum, sizeof(maximum), "\"%ju\"", UINTMAX_MAX);
    assert(length > 0 && (size_t)length < sizeof(maximum));
    prepare(maximum); uintmax_t number = 0;
    SourceJsonStatus status = source_json_schema_unsigned(root, &number, &issue);
    assert(status == SOURCE_JSON_OK && number == UINTMAX_MAX); finish();
    maximum[length - 1] = '0'; maximum[length] = '"'; maximum[length + 1] = 0;
    const char *invalid[] = {maximum, "1", "\"-1\"", "\"01\""};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        prepare(invalid[index]); number = 99;
        status = source_json_schema_unsigned(root, &number, &issue);
        assert(status == SOURCE_JSON_INVALID && number == 99); finish();
    }
}
/** C-string extraction preserves original spacing and refuses truncation at an
 * escaped NUL; path/name semantics belong to the later domain layer. */
static void it_extracts_nonempty_nul_free_strings_without_trimming(void) {
    prepare("\" x \""); const char *text = NULL;
    SourceJsonStatus status = source_json_schema_cstring(root, &text, &issue);
    assert(status == SOURCE_JSON_OK && !strcmp(text, " x ")); finish();
    const char *invalid[] = {"\"\"", "\"a\\u0000b\"", "false", "null", "1"};
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        prepare(invalid[index]); const char *sentinel = "sentinel"; text = sentinel;
        status = source_json_schema_cstring(root, &text, &issue);
        assert(status == SOURCE_JSON_INVALID && text == sentinel && issue.origin.source == input); finish();
    }
}
/** The sole payload allocation can fail without publishing an incomplete byte
 * view, and all syntax/payload scratch is reclaimed independently. */
static void it_preserves_payload_output_on_allocation_failure_and_invalid_owners(void) {
    prepare("{\"utf8\":\"a\",\"hex\":\"61\"}");
    SourceJsonText bytes = {(const unsigned char *)"sentinel", 8, {input, 0, 0}};
    size_t before = allocation_count; failure_at = before + 1;
    SourceJsonStatus status = source_json_schema_bytes(&decoded, root, &bytes, &issue);
    assert(status == SOURCE_JSON_OUT_OF_MEMORY && allocation_count == before + 1);
    assert(bytes.length == 8 && !memcmp(bytes.bytes, "sentinel", 8) && issue.message);
    failure_at = 0;
    assert(source_json_schema_bytes(NULL, root, &bytes, &issue) == SOURCE_JSON_INVALID);
    assert(source_json_schema_bytes(&decoded, root, NULL, &issue) == SOURCE_JSON_INVALID);
    assert(source_json_schema_bytes(&decoded, root, &bytes, NULL) == SOURCE_JSON_INVALID);
    size_t size = 99; const char *text = "sentinel";
    assert(source_json_schema_size(root, &size, NULL) == SOURCE_JSON_INVALID && size == 99);
    assert(source_json_schema_cstring(NULL, &text, &issue) == SOURCE_JSON_INVALID && !strcmp(text, "sentinel"));
    assert(source_json_schema_fields(NULL, NULL, 0, NULL, &issue) == SOURCE_JSON_INVALID);
    finish();
}
/** Execute exact schema, bytes, integer, string and failed-owner contracts. */
void run(void) {
    it_matches_exact_unordered_fields_without_prefixes_or_partial_publication();
    it_decodes_all_byte_values_and_consistent_unicode_views_exactly();
    it_refuses_noncanonical_or_inconsistent_byte_objects_without_publication();
    it_checks_canonical_decimal_strings_at_size_boundaries();
    it_retains_full_width_unsigned_identity_strings();
    it_extracts_nonempty_nul_free_strings_without_trimming();
    it_preserves_payload_output_on_allocation_failure_and_invalid_owners();
}
/** Run with assertions enabled under the component Makefile. */
int main(void) { run(); return 0; }
