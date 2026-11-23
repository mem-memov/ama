/** @file
 * @brief Roundtrip exact filesystem facts and reject malformed records, every allocation and output failure.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#define _GNU_SOURCE
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/transaction/document/document.h"
#include "../../../../../../tools/project/source/observation/freshness.h"
static SourceArena arena;
static SourceManager records;
static SourceCatalogImage image;
static SourceCatalogImageEntry rows[5];
static ModuleRoot root;
/** Build binary, empty, unread and unknown observations with exact large identities. */
static void begin(void) {
    setup_test(); source_arena_init(&arena, sources.allocator); source_manager_init(&records, sources.allocator);
    static const char *excluded[] = {"", "ordinary", "\xff"};
    root = (ModuleRoot){.path = "/tree", .kind = MODULE_ROOT_PROJECT, .excluded = excluded, .excluded_count = 3};
    const char *paths[] = {"/tree", "/tree/\xff", "/tree/empty", "/tree/ignored", "/tree/unread"};
    for (size_t i = 0; i < 5; i++) {
        rows[i] = (SourceCatalogImageEntry){.path = paths[i], .metadata = {.st_dev = 3, .st_ino = (ino_t)(UINT64_C(9007199254740993) + i), .st_mode = (i ? S_IFREG : S_IFDIR) | 0700},
            .metadata_captured = i != 3, .children_complete = i == 0};
    }
    const unsigned char binary[] = {0, 255, 195};
    assert(source_manager_add(&sources, paths[1], binary, sizeof(binary), &rows[1].content) == SOURCE_OK);
    assert(source_manager_add(&sources, paths[2], NULL, 0, &rows[2].content) == SOURCE_OK);
    SourceImageIssue issue;
    assert(source_catalog_image_prepare(&image, rows, 5, &root, 1, &issue) == SOURCE_IMAGE_OK);
}
/** Keep retained image owners alive until all independent epoch consumers retire. */
static void end(void) {
    source_manager_destroy(&records); source_arena_destroy(&arena); destroy_test();
}
/** Read a borrowed flushed temporary stream as exact JSON source bytes. */
static const SourceText *record(FILE *stream) {
    long length = ftell(stream); assert(length >= 0 && fseek(stream, 0, SEEK_SET) == 0);
    unsigned char *bytes = source_arena_allocate(&arena, (size_t)length); assert(bytes);
    assert(fread(bytes, 1, (size_t)length, stream) == (size_t)length);
    const SourceText *text = NULL;
    assert(source_manager_add(&records, "image-record.json", bytes, (size_t)length, &text) == SOURCE_OK); return text;
}
/** Encode through the production writer and a real borrowed stream. */
static const SourceText *encoded(void) {
    FILE *stream = tmpfile(); assert(stream);
    assert(source_image_document_write(stream, &image)); const SourceText *text = record(stream);
    assert(fclose(stream) == 0); return text;
}
/** Paths, bytes, empty/unknown precision and exact integer fields survive syntax
 * owner expiry; separate before/after epochs allow different bytes at one path. */
static void it_roundtrips_lossless_images_and_independent_content_epochs(void) {
    begin(); const SourceText *first = encoded(); SourceImageDocument before, after;
    source_image_document_init(&before, sources.allocator); source_image_document_init(&after, sources.allocator);
    assert(source_image_document_read(&before, first) == SOURCE_JSON_OK && before.valid);
    SourceFreshnessIssue issue;
    assert(source_observation_freshness(&image, &before.image, &issue) == SOURCE_FRESH_OK);
    assert(source_image_document_read(&before, first) == SOURCE_JSON_INVALID && before.valid);
    assert(before.image.entries[2].content && !source_length(before.image.entries[2].content) && !before.image.entries[4].content);
    assert(!before.image.entries[3].metadata_captured && !strcmp(before.image.roots[0].excluded[2], "\xff"));
    assert(source_manager_add(&sources, "/tree/empty", (const unsigned char *)"after", 5, &rows[2].content) == SOURCE_OK);
    const SourceText *second = encoded();
    assert(source_image_document_read(&after, second) == SOURCE_JSON_OK && after.valid);
    source_manager_destroy(&records); source_manager_init(&records, sources.allocator);
    assert(!source_length(before.image.entries[2].content) && source_length(after.image.entries[2].content) == 5);
    assert(source_observation_freshness(&image, &after.image, &issue) == SOURCE_FRESH_OK);
    source_image_document_destroy(&after); source_image_document_destroy(&before);
    SourceCatalogImage empty; SourceImageIssue shape;
    assert(source_catalog_image_prepare(&empty, NULL, 0, NULL, 0, &shape) == SOURCE_IMAGE_OK);
    image = empty; const SourceText *empty_record = encoded(); source_image_document_init(&before, sources.allocator);
    assert(source_image_document_read(&before, empty_record) == SOURCE_JSON_OK && !before.image.count && !before.image.root_count);
    source_image_document_destroy(&before); end();
}
/** Closed records reject future/missing/duplicate fields, invalid scalar forms,
 * NUL/invalid byte paths and malformed topology without publishing valid facts. */
static void it_refuses_invalid_image_records_and_preserves_failed_epoch_ownership(void) {
    static const char *cases[] = {
        "{}", "{\"format\":\"other\",\"roots\":[],\"entries\":[]}",
        "{\"format\":\"ama.source-image.v1\",\"format\":\"ama.source-image.v1\",\"roots\":[],\"entries\":[]}",
        "{\"format\":\"ama.source-image.v1\",\"roots\":[],\"entries\":[],\"extra\":false}",
        "{\"format\":\"ama.source-image.v1\",\"roots\":false,\"entries\":[]}",
        "{\"format\":\"ama.source-image.v1\",\"roots\":[],\"entries\":false}",
        "{\"format\":\"ama.source-image.v1\",\"roots\":[{\"path\":{\"utf8\":\"/\",\"hex\":\"2f\"},\"kind\":\"other\",\"excluded\":[]}],\"entries\":[]}",
        "{\"format\":\"ama.source-image.v1\",\"roots\":[],\"entries\":[{\"path\":{\"utf8\":\"/x\",\"hex\":\"2f78\"},\"metadata\":null,\"childrenComplete\":false,\"content\":null}]}",
        "{\"format\":\"ama.source-image.v1\",\"roots\":[],\"entries\":[{\"path\":null,\"metadata\":null,\"childrenComplete\":false,\"content\":null}]}",
        "{\"format\":\"ama.source-image.v1\",\"roots\":[],\"entries\":[{\"path\":{\"utf8\":\"/x\",\"hex\":\"2F78\"},\"metadata\":null,\"childrenComplete\":false,\"content\":null}]}",
        "{\"format\":\"ama.source-image.v1\",\"roots\":[],\"entries\":[{\"path\":{\"utf8\":\"/x\",\"hex\":\"2f78\"},\"metadata\":{\"device\":1,\"inode\":\"1\",\"mode\":\"1\"},\"childrenComplete\":false,\"content\":null}]}",
        "{\"format\":\"ama.source-image.v1\",\"roots\":[],\"entries\":[{\"path\":{\"utf8\":\"/x\",\"hex\":\"2f78\"},\"metadata\":{\"device\":\"1\",\"inode\":\"18446744073709551616\",\"mode\":\"1\"},\"childrenComplete\":false,\"content\":null}]}"
    };
    begin();
    for (size_t i = 0; i < sizeof(cases) / sizeof(*cases); i++) {
        const SourceText *text = NULL;
        assert(source_manager_add(&records, "bad-record.json", (const unsigned char *)cases[i], strlen(cases[i]), &text) == SOURCE_OK);
        SourceImageDocument document; source_image_document_init(&document, sources.allocator);
        assert(source_image_document_read(&document, text) == SOURCE_JSON_INVALID && !document.valid && document.issue.message);
        source_image_document_destroy(&document);
    }
    end();
}
/** Every parser/path/row/content/topology allocation failure stays independently
 * owned and never publishes a valid partial epoch. */
static void it_retires_every_image_record_allocation_failure(void) {
    begin(); const SourceText *text = encoded(); size_t total = 0, live = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceImageDocument document; source_image_document_init(&document, sources.allocator); size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceJsonStatus status = source_image_document_read(&document, text);
        if (!failure) { assert(status == SOURCE_JSON_OK); total = allocation_count - before; assert(total); }
        else { assert(status == SOURCE_JSON_OUT_OF_MEMORY && !document.valid); }
        failure_at = 0; source_image_document_destroy(&document); assert(live_allocations == live);
    }
    end();
}
/** Real stdio callback failures exercise error propagation without an unwritable
 * host device or permission-dependent test skip. */
typedef struct { size_t calls, failure, closes; } OutputFailure;
/** Accept full output or fail one exact write boundary with ENOSPC. */
static ssize_t write_cookie(void *context, const char *bytes, size_t count) {
    OutputFailure *failure = context; (void)bytes; failure->calls++;
    if (failure->calls == failure->failure) { errno = ENOSPC; return -1; }
    assert(count <= SSIZE_MAX); return (ssize_t)count;
}
/** The caller remains responsible for closing its borrowed stream on failure. */
static int close_cookie(void *context) { ((OutputFailure *)context)->closes++; return 0; }
/** Every measured stream write failure refuses a complete encoded record. */
static void it_reports_every_image_record_output_failure_and_borrows_stream_ownership(void) {
    begin(); size_t calls = 0;
    for (size_t failure = 0; !failure || failure <= calls; failure++) {
        OutputFailure output = {.failure = failure}; cookie_io_functions_t functions = {.write = write_cookie, .close = close_cookie};
        FILE *stream = fopencookie(&output, "w", functions); assert(stream && setvbuf(stream, NULL, _IONBF, 0) == 0);
        bool written = source_image_document_write(stream, &image);
        if (!failure) { assert(written); calls = output.calls; assert(calls); }
        else { assert(!written && ferror(stream)); }
        (void)fclose(stream); assert(output.closes == 1);
    }
    assert(!source_image_document_write(NULL, &image)); end();
}
/** Explicit finite schema, byte/identity, resource and host-stream cases. */
void run(void) {
    it_roundtrips_lossless_images_and_independent_content_epochs();
    it_refuses_invalid_image_records_and_preserves_failed_epoch_ownership();
    it_retires_every_image_record_allocation_failure();
    it_reports_every_image_record_output_failure_and_borrows_stream_ownership();
}
/** Finish only after all independent epoch and stream owners retire. */
int main(void) { run(); return 0; }
