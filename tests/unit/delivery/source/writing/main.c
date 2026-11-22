/** @file
 * @brief Verify exact borrowed capture serialization without a forged model-valid flag or stream ownership.
 * @see [T273 contract](../../../../../docs/implementation/source-protocol.md)
 */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../../source/common.c"
#include "delivery/source/snapshot/write.h"
static DeliverySourceSnapshot snapshot;
/** Invoke only controlled independent fixture/verification code. */
static void python(const char *script) {
    pid_t child = fork(); assert(child >= 0);
    if (!child) { execlp("python3", "python3", script, (char *)NULL); _exit(127); }
    int status;
    assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Parse an independently encoded capture into a healthy borrowed view. */
static void prepare(void) {
    python("../snapshot/generate.py"); setup_test();
    delivery_source_snapshot_init(&snapshot, sources.allocator);
    const SourceText *input = NULL;
    SourceStatus loaded = source_manager_load(&sources, "generated/base.json", &input); assert(loaded == SOURCE_OK);
    SourceJsonStatus parsed = delivery_source_snapshot_read(&snapshot, input); assert(parsed == SOURCE_JSON_OK);
}
/** Retire captured source data before its independent original request. */
static void finish(void) { delivery_source_snapshot_destroy(&snapshot); destroy_test(); }
/** Every field/byte/integer is independently compared, including opaque draft
 * ABI data and non-UTF-8 physical paths. */
static void it_serializes_exact_borrowed_capture_authority_without_mutation(void) {
    prepare(); FILE *output = fopen("generated/roundtrip.json", "wb"); assert(output);
    bool written = delivery_source_snapshot_write(output, &(DeliverySourceSnapshotView){snapshot.entries, snapshot.count, snapshot.versions, snapshot.companions});
    assert(written && fclose(output) == 0); python("verify.py");
    assert(source_bytes(snapshot.entries[0].text)[0] == '@' && snapshot.entries[0].device == (dev_t)7);
    finish();
}
/** Empty input creates only explicit empty arrays and roundtrips through the
 * normal decoder, without synthesizing a source or version. */
static void it_serializes_empty_capture_without_synthetic_rows(void) {
    prepare(); FILE *output = fopen("generated/empty-written.json", "wb"); assert(output);
    bool written = delivery_source_snapshot_write(output, &(DeliverySourceSnapshotView){NULL, 0, NULL, NULL});
    assert(written && fclose(output) == 0);
    const SourceText *input = NULL;
    SourceStatus loaded = source_manager_load(&sources, "generated/empty-written.json", &input); assert(loaded == SOURCE_OK);
    DeliverySourceSnapshot empty; delivery_source_snapshot_init(&empty, sources.allocator);
    assert(delivery_source_snapshot_read(&empty, input) == SOURCE_JSON_OK && !empty.count && !empty.versions);
    delivery_source_snapshot_destroy(&empty); finish();
}
/** Missing/duplicate/foreign companion authority and repeated identities refuse
 * before writing any request byte, never selecting a convenient first row. */
static void it_refuses_incomplete_or_ambiguous_capture_views_before_output(void) {
    prepare(); FILE *output = tmpfile(); assert(output);
    assert(!delivery_source_snapshot_write(output, &(DeliverySourceSnapshotView){NULL, 1, snapshot.versions, snapshot.companions}));
    assert(!delivery_source_snapshot_write(output, &(DeliverySourceSnapshotView){snapshot.entries, snapshot.count, snapshot.versions, NULL}));
    DeliverySourceCompanions duplicate = *snapshot.companions;
    duplicate.next = (DeliverySourceCompanions *)snapshot.companions;
    assert(!delivery_source_snapshot_write(output, &(DeliverySourceSnapshotView){snapshot.entries, snapshot.count, snapshot.versions, &duplicate}));
    duplicate.identity = "//foreign";
    assert(!delivery_source_snapshot_write(output, &(DeliverySourceSnapshotView){snapshot.entries, snapshot.count, snapshot.versions, &duplicate}));
    duplicate.identity = NULL;
    assert(!delivery_source_snapshot_write(output, &(DeliverySourceSnapshotView){snapshot.entries, snapshot.count, snapshot.versions, &duplicate}));
    assert(snapshot.count == 4);
    ModuleSnapshotEntry entries[4]; memcpy(entries, snapshot.entries, sizeof(entries));
    entries[1].logical_path = entries[0].logical_path;
    assert(!delivery_source_snapshot_write(output, &(DeliverySourceSnapshotView){entries, 4, snapshot.versions, snapshot.companions}));
    memcpy(entries, snapshot.entries, sizeof(entries)); entries[0].text = NULL;
    assert(!delivery_source_snapshot_write(output, &(DeliverySourceSnapshotView){entries, 4, snapshot.versions, snapshot.companions}));
    DeliverySnapshotVersion version = *snapshot.versions;
    version.next = (DeliverySnapshotVersion *)snapshot.versions;
    assert(!delivery_source_snapshot_write(output, &(DeliverySourceSnapshotView){snapshot.entries, snapshot.count, &version, snapshot.companions}));
    assert(ftell(output) == 0 && fclose(output) == 0);
    finish();
}
/** Immediate write and final flush failures remain failures and do not close
 * streams or alter input captures. */
static void it_propagates_capture_writer_stream_and_flush_failures(void) {
    prepare(); FILE *output = fopen("main.c", "rb"); assert(output);
    assert(!delivery_source_snapshot_write(output, &(DeliverySourceSnapshotView){snapshot.entries, snapshot.count, snapshot.versions, snapshot.companions}));
    assert(fseek(output, 0, SEEK_SET) == 0 && fclose(output) == 0);
    output = fopen("/dev/full", "wb"); assert(output);
    assert(!delivery_source_snapshot_write(output, &(DeliverySourceSnapshotView){snapshot.entries, snapshot.count, snapshot.versions, snapshot.companions}));
    (void)fclose(output);
    assert(!delivery_source_snapshot_write(NULL, &(DeliverySourceSnapshotView){snapshot.entries, snapshot.count, snapshot.versions, snapshot.companions}));
    finish();
}
/** Execute independent wire comparison and authority/stream refusal contracts. */
void run(void) {
    it_serializes_exact_borrowed_capture_authority_without_mutation();
    it_serializes_empty_capture_without_synthetic_rows();
    it_refuses_incomplete_or_ambiguous_capture_views_before_output();
    it_propagates_capture_writer_stream_and_flush_failures();
}
/** Preserve generated evidence on any failed assertion or child verifier. */
int main(void) { run(); return 0; }
