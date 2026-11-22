/** @file
 * @brief Verify exact captured request data, ownership, malformed models and every allocation failure.
 * @see [T224 contract](../../../../../docs/implementation/source-operations.md)
 */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../../source/common.c"
#include "delivery/source/snapshot/snapshot.h"
static DeliverySourceSnapshot snapshot;
static SourceManager request;
static const SourceText *input;
/** Python's independent JSON encoder creates controlled fixture bytes only. */
static void generate(void) {
    pid_t child = fork(); assert(child >= 0);
    if (!child) { execlp("python3", "python3", "generate.py", (char *)NULL); _exit(127); }
    int status;
    assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Load a fixture request into a separate owner so successful model independence
 * from original JSON storage can be verified directly. */
static void prepare(const char *name) {
    setup_test(); source_manager_init(&request, sources.allocator);
    delivery_source_snapshot_init(&snapshot, sources.allocator);
    char path[256]; int length = snprintf(path, sizeof(path), "generated/%s", name);
    assert(length > 0 && (size_t)length < sizeof(path));
    SourceStatus loaded = source_manager_load(&request, path, &input); assert(loaded == SOURCE_OK);
}
/** All partial/result records retire before their original diagnostic source. */
static void finish(void) {
    delivery_source_snapshot_destroy(&snapshot); source_manager_destroy(&request); destroy_test();
}
/** Reuse an exact identity only after checking the captured row list. */
static const DeliverySourceCompanions *companions(const char *identity) {
    for (const DeliverySourceCompanions *row = snapshot.companions; row; row = row->next) {
        if (!strcmp(row->identity, identity)) { return row; }
    }
    assert(false); return NULL;
}
/** Unicode-invalid physical bytes, selected version identity, repeated coherent
 * configuration and opaque draft ABI data remain exact owned snapshots. */
static void it_decodes_owned_version_source_and_unconditional_companion_data(void) {
    generate(); prepare("base.json");
    SourceJsonStatus status = delivery_source_snapshot_read(&snapshot, input);
    assert(status == SOURCE_JSON_OK && snapshot.valid && snapshot.attempted && snapshot.count == 4);
    assert(snapshot.versions && !snapshot.versions->next);
    assert(!strcmp(snapshot.versions->identity, "/vendor/pkg/v1.0.0"));
    assert(!strcmp(snapshot.versions->directory, "not-published/version"));
    assert(!strcmp(snapshot.entries[0].logical_path, "/vendor/pkg/v1.0.0/src/main"));
    assert(!strcmp(source_path(snapshot.entries[0].text), "not-published/\xffmain"));
    assert(snapshot.entries[0].device == (dev_t)7 && snapshot.entries[0].inode == (ino_t)1);
    assert(snapshot.entries[1].configuration == snapshot.versions->configuration);
    const DeliverySourceCompanions *native = companions("//native");
    assert(!strcmp(native->body, "not-published/native.c") && !strcmp(native->header, "not-published/native.h"));
    assert(source_length(native->abi) == strlen("+ initialize\n- destroy\n"));
    const DeliverySourceCompanions *draft = companions("/vendor/pkg/v1.0.0/src/draft");
    assert(source_length(draft->abi) == 2 && source_bytes(draft->abi)[0] == 0xff && source_bytes(draft->abi)[1] == 0);
    source_manager_destroy(&request);
    assert(!strcmp(snapshot.entries[0].logical_path, "/vendor/pkg/v1.0.0/src/main"));
    assert(source_bytes(snapshot.entries[0].text)[0] == '@' && source_bytes(draft->abi)[0] == 0xff);
    finish();
}
/** Data decoding never substitutes for actual source/ABI/alias checks; these
 * structurally valid inputs must remain available to the ordinary frontend. */
static void it_defers_source_native_and_physical_alias_checks_to_the_frontend(void) {
    generate();
    const char *names[] = {"bad-source.json", "bad-source-utf8.json", "bad-native-abi.json", "aliased-metadata.json"};
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        prepare(names[index]);
        SourceJsonStatus status = delivery_source_snapshot_read(&snapshot, input);
        assert(status == SOURCE_JSON_OK && snapshot.valid);
        finish();
    }
}
/** Every one-property malformed input refuses before its model is marked valid. */
static void it_refuses_closed_schema_and_capture_coherence_errors(void) {
    generate(); FILE *manifest = fopen("generated/invalid.txt", "r"); assert(manifest);
    char name[128]; size_t count = 0;
    while (fscanf(manifest, "%127s", name) == 1) {
        prepare(name);
        SourceJsonStatus status = delivery_source_snapshot_read(&snapshot, input);
        assert(status == SOURCE_JSON_INVALID && snapshot.attempted && !snapshot.valid);
        assert(snapshot.issue.message && snapshot.issue.origin.source == input && source_span_valid(snapshot.issue.origin));
        finish(); count++;
    }
    assert(!ferror(manifest) && fclose(manifest) == 0 && count == 27);
}
/** Each temporary JSON, result name/row/source and companion allocation can fail
 * independently; all retained and transient storage is retired exactly once. */
static void it_retires_every_failed_snapshot_decode_allocation(void) {
    generate(); size_t allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        prepare("base.json"); size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceJsonStatus status = delivery_source_snapshot_read(&snapshot, input);
        if (!failure) { assert(status == SOURCE_JSON_OK); allocations = allocation_count - before; assert(allocations); }
        else {
            assert(status == SOURCE_JSON_OUT_OF_MEMORY && snapshot.attempted && !snapshot.valid);
            assert(snapshot.issue.message && snapshot.issue.origin.source == input);
        }
        failure_at = 0; finish();
    }
}
/** Empty snapshots create no phantom source, and repeated/missing input owners
 * cannot replace a previously accepted result. */
static void it_accepts_empty_input_and_refuses_owner_reuse_without_replacing_results(void) {
    generate(); prepare("empty.json");
    assert(delivery_source_snapshot_read(NULL, input) == SOURCE_JSON_INVALID);
    assert(delivery_source_snapshot_read(&snapshot, NULL) == SOURCE_JSON_INVALID && !snapshot.attempted);
    SourceJsonStatus status = delivery_source_snapshot_read(&snapshot, input);
    assert(status == SOURCE_JSON_OK && snapshot.valid && !snapshot.count && !snapshot.entries && !snapshot.versions);
    status = delivery_source_snapshot_read(&snapshot, input);
    assert(status == SOURCE_JSON_INVALID && snapshot.valid && snapshot.status == SOURCE_JSON_OK);
    finish();
}
/** Execute independent data, malformed-model, source ownership and fault cases. */
void run(void) {
    it_decodes_owned_version_source_and_unconditional_companion_data();
    it_defers_source_native_and_physical_alias_checks_to_the_frontend();
    it_refuses_closed_schema_and_capture_coherence_errors();
    it_retires_every_failed_snapshot_decode_allocation();
    it_accepts_empty_input_and_refuses_owner_reuse_without_replacing_results();
}
/** Preserve fixture evidence if any assertion or child decoder fails. */
int main(void) { run(); return 0; }
