/** @file
 * @brief Check lossless shared source captures, ownership separation and every primitive failure.
 * @see [T273 contract](../../../../../docs/implementation/source-protocol.md)
 */
#include "../../common.c"
#include "source/json/capture/capture.h"
static SourceArena syntax, scratch;
static SourceJsonCapture capture;
static const SourceJsonValue *root;
static SourceJsonIssue issue;
/** Retain separate input, syntax, byte scratch and result owners. */
static void prepare(const char *text) {
    setup_test(); source_arena_init(&syntax, sources.allocator);
    source_arena_init(&scratch, sources.allocator); source_json_capture_init(&capture, sources.allocator);
    const SourceText *input = NULL;
    SourceStatus added = source_manager_add(&sources, "response.json", (const unsigned char *)text, strlen(text), &input);
    assert(added == SOURCE_OK); issue = (SourceJsonIssue){0}; root = NULL;
    SourceJsonStatus status = source_json_parse(&syntax, input, &root, &issue); assert(status == SOURCE_JSON_OK);
}
/** Release retained source data after observing independence from scratch/input. */
static void finish(void) {
    source_json_capture_destroy(&capture); source_arena_destroy(&scratch);
    source_arena_destroy(&syntax); destroy_test();
}
/** Equal physical byte paths reuse exact snapshots, while conflicting content
 * cannot replace previous data or publish a partial result. */
static void it_reuses_coherent_binary_snapshots_and_refuses_conflicting_paths(void) {
    prepare("[{\"path\":{\"utf8\":null,\"hex\":\"702fff\"},\"content\":{\"utf8\":null,\"hex\":\"ff00\"}},"
        "{\"path\":{\"utf8\":null,\"hex\":\"702fff\"},\"content\":{\"utf8\":null,\"hex\":\"ff00\"}},"
        "{\"path\":{\"utf8\":null,\"hex\":\"702fff\"},\"content\":{\"utf8\":null,\"hex\":\"fe00\"}}]");
    const SourceText *first = NULL, *same = NULL;
    SourceJsonStatus status = source_json_capture_text(&capture, &scratch, root->first, &first, &issue);
    assert(status == SOURCE_JSON_OK && source_length(first) == 2 && source_bytes(first)[0] == 0xff);
    assert(!strcmp(source_path(first), "p/\xff"));
    status = source_json_capture_text(&capture, &scratch, root->first->next, &same, &issue);
    assert(status == SOURCE_JSON_OK && same == first);
    status = source_json_capture_text(&capture, &scratch, root->first->next->next, &same, &issue);
    assert(status == SOURCE_JSON_INVALID && same == first && issue.message);
    source_arena_destroy(&scratch); source_arena_destroy(&syntax); source_manager_destroy(&sources);
    assert(source_bytes(first)[0] == 0xff && source_bytes(first)[1] == 0 && !strcmp(source_path(first), "p/\xff"));
    finish();
}
/** String spacing, invalid-UTF-8 physical paths and observed null remain
 * distinct values without trimming, replacement or source-role inference. */
static void it_retains_strings_raw_paths_and_explicit_absence(void) {
    prepare("[\" name \",{\"utf8\":null,\"hex\":\"702fff\"},null]");
    const char *name = NULL, *path = NULL;
    assert(source_json_capture_string(&capture, root->first, &name, &issue) == SOURCE_JSON_OK);
    assert(!strcmp(name, " name "));
    assert(source_json_capture_path(&capture, &scratch, root->first->next, &path, &issue) == SOURCE_JSON_OK);
    assert(!strcmp(path, "p/\xff"));
    assert(source_json_capture_path(&capture, &scratch, root->first->next->next, &path, &issue) == SOURCE_JSON_OK && !path);
    const SourceText *text = root->origin.source;
    assert(source_json_capture_text(&capture, &scratch, root->first->next->next, &text, &issue) == SOURCE_JSON_OK && !text);
    finish();
}
/** Empty/NUL paths and incorrectly shaped blobs are refused without changing
 * the caller's existing output. */
static void it_refuses_truncated_paths_invalid_strings_and_malformed_blobs(void) {
    const char *paths[] = {"{\"utf8\":\"\",\"hex\":\"\"}", "{\"utf8\":\"\\u0000\",\"hex\":\"00\"}", "true"};
    for (size_t index = 0; index < sizeof(paths) / sizeof(*paths); index++) {
        prepare(paths[index]); const char *sentinel = "sentinel", *path = sentinel;
        assert(source_json_capture_path(&capture, &scratch, root, &path, &issue) == SOURCE_JSON_INVALID && path == sentinel);
        finish();
    }
    prepare("\"a\\u0000b\""); const char *name = "sentinel";
    assert(source_json_capture_string(&capture, root, &name, &issue) == SOURCE_JSON_INVALID && !strcmp(name, "sentinel"));
    finish();
    const char *blobs[] = {"{\"path\":null,\"content\":{\"utf8\":\"\",\"hex\":\"\"}}", "\"invalid\"", "{}"};
    for (size_t index = 0; index < sizeof(blobs) / sizeof(*blobs); index++) {
        prepare(blobs[index]); const SourceText *text = root->origin.source;
        const SourceText *sentinel = text;
        assert(source_json_capture_text(&capture, &scratch, root, &text, &issue) == SOURCE_JSON_INVALID && text == sentinel);
        finish();
    }
}
/** Every decoded-byte/path/index/SourceText allocation can fail independently
 * and all partial storage remains owned without publishing a source. */
static void it_preserves_output_on_every_capture_allocation_failure(void) {
    const char *text = "{\"path\":{\"utf8\":\"file\",\"hex\":\"66696c65\"},\"content\":{\"utf8\":\"data\",\"hex\":\"64617461\"}}";
    size_t allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        prepare(text); const SourceText *output = root->origin.source, *sentinel = output;
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceJsonStatus status = source_json_capture_text(&capture, &scratch, root, &output, &issue);
        if (!failure) { assert(status == SOURCE_JSON_OK); allocations = allocation_count - before; assert(allocations); }
        else { assert(status == SOURCE_JSON_OUT_OF_MEMORY && output == sentinel && issue.message); }
        failure_at = 0; finish();
    }
}
/** A missing or aliased scratch owner cannot accidentally own retained result
 * names or allow later scratch cleanup to destroy accepted data. */
static void it_refuses_missing_or_aliased_capture_owners(void) {
    prepare("null"); const char *path = "sentinel"; const SourceText *text = root->origin.source;
    assert(source_json_capture_path(NULL, &scratch, root, &path, &issue) == SOURCE_JSON_INVALID);
    assert(source_json_capture_path(&capture, NULL, root, &path, &issue) == SOURCE_JSON_INVALID);
    assert(source_json_capture_path(&capture, &capture.arena, root, &path, &issue) == SOURCE_JSON_INVALID);
    assert(source_json_capture_text(&capture, &capture.arena, root, &text, &issue) == SOURCE_JSON_INVALID);
    assert(source_json_capture_text(&capture, &scratch, NULL, &text, &issue) == SOURCE_JSON_INVALID);
    assert(source_json_capture_string(&capture, NULL, &path, &issue) == SOURCE_JSON_INVALID);
    assert(source_json_capture_string(&capture, root, NULL, &issue) == SOURCE_JSON_INVALID);
    assert(source_json_capture_string(&capture, root, &path, NULL) == SOURCE_JSON_INVALID);
    assert(!strcmp(path, "sentinel") && text == root->origin.source);
    finish();
}
/** Execute coherent capture, independent lifetime, schema and allocation checks. */
void run(void) {
    it_reuses_coherent_binary_snapshots_and_refuses_conflicting_paths();
    it_retains_strings_raw_paths_and_explicit_absence();
    it_refuses_truncated_paths_invalid_strings_and_malformed_blobs();
    it_preserves_output_on_every_capture_allocation_failure();
    it_refuses_missing_or_aliased_capture_owners();
}
/** Assertions remain active for this independently runnable component. */
int main(void) { run(); return 0; }
