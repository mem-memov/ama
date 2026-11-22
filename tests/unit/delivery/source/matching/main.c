/** @file
 * @brief Reject stale/missing/ambiguous compiler observations before graph admission.
 * @see [T273 contract](../../../../../docs/implementation/source-protocol.md)
 */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../../source/common.c"
#include "delivery/source/response/match.h"
static DeliverySourceResponse response;
static DeliverySourceSnapshot snapshot;
static DeliverySourceSnapshotView view;
/** Use independently specified request and response models. */
static void prepare(void) {
    pid_t child = fork(); assert(child >= 0);
    if (!child) { execlp("python3", "python3", "../response/generate.py", (char *)NULL); _exit(127); }
    int status; assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    setup_test();
    delivery_source_snapshot_init(&snapshot, sources.allocator);
    delivery_source_response_init(&response, sources.allocator);
    const SourceText *input = NULL;
    assert(source_manager_load(&sources, "generated/base.json", &input) == SOURCE_OK);
    assert(delivery_source_snapshot_read(&snapshot, input) == SOURCE_JSON_OK);
    assert(source_manager_load(&sources, "generated/captured-response.json", &input) == SOURCE_OK);
    assert(delivery_source_response_read(&response, input) == SOURCE_JSON_OK);
    view = (DeliverySourceSnapshotView){snapshot.entries, snapshot.count, snapshot.versions, snapshot.companions};
}
/** Independent result owners retire before fixture allocator. */
static void finish(void) {
    delivery_source_response_destroy(&response); delivery_source_snapshot_destroy(&snapshot); destroy_test();
}
/** Exact byte snapshots and captured absence match without allocation. */
static void it_accepts_complete_exact_capture_and_empty_domains(void) {
    prepare(); size_t before = allocation_count;
    assert(delivery_source_response_matches(&response, &view) && allocation_count == before);
    ModuleSnapshotEntry reversed[4]; assert(view.count == 4);
    for (size_t index = 0; index < 4; index++) { reversed[index] = view.entries[3 - index]; }
    view.entries = reversed; assert(delivery_source_response_matches(&response, &view));
    DeliverySourceResponse empty = {.valid = true};
    assert(delivery_source_response_matches(&empty, &(DeliverySourceSnapshotView){0}));
    assert(!delivery_source_response_matches(NULL, &view));
    assert(!delivery_source_response_matches(&response, NULL));
    empty.valid = false; assert(!delivery_source_response_matches(&empty, &view)); finish();
}
/** Cardinality, exact identity, bytes and capture observation are all necessary. */
static void it_refuses_changed_missing_duplicate_or_unobserved_sources(void) {
    prepare(); ModuleSnapshotEntry entries[4]; assert(view.count == 4);
    memcpy(entries, view.entries, sizeof(entries)); view.entries = entries;
    view.count--; assert(!delivery_source_response_matches(&response, &view)); view.count++;
    entries[0].logical_path = entries[1].logical_path;
    assert(!delivery_source_response_matches(&response, &view));
    memcpy(entries, snapshot.entries, sizeof(entries)); entries[0].text = entries[1].text;
    assert(!delivery_source_response_matches(&response, &view));
    memcpy(entries, snapshot.entries, sizeof(entries)); entries[0].configuration = entries[1].configuration;
    assert(!delivery_source_response_matches(&response, &view));
    view.entries = snapshot.entries;
    DeliveryResponseSource *row = (DeliveryResponseSource *)response.sources;
    row->configuration_captured = false; assert(!delivery_source_response_matches(&response, &view));
    row->configuration_captured = true; assert(delivery_source_response_matches(&response, &view)); finish();
}
/** Native observations compare to exactly one authority, without examining C/ABI syntax. */
static void it_refuses_changed_or_ambiguous_companion_authority(void) {
    prepare(); DeliverySourceCompanions duplicate = *view.companions;
    duplicate.next = (DeliverySourceCompanions *)view.companions; view.companions = &duplicate;
    assert(!delivery_source_response_matches(&response, &view));
    duplicate.identity = "//foreign"; assert(!delivery_source_response_matches(&response, &view));
    view.companions = snapshot.companions;
    DeliverySourceCompanions *native = (DeliverySourceCompanions *)snapshot.companions;
    while (strcmp(native->identity, "//native")) { native = native->next; }
    const char *body = native->body; native->body = "changed.c";
    assert(!delivery_source_response_matches(&response, &view)); native->body = body;
    const char *header = native->header; native->header = "changed.h";
    assert(!delivery_source_response_matches(&response, &view)); native->header = header;
    const SourceText *abi = native->abi; native->abi = NULL;
    assert(!delivery_source_response_matches(&response, &view)); native->abi = abi;
    assert(delivery_source_response_matches(&response, &view)); finish();
}
/** Version identity, directory and configuration are independently authoritative. */
static void it_refuses_changed_missing_duplicate_versions(void) {
    prepare(); DeliverySnapshotVersion version = *view.versions;
    view.versions = NULL; assert(!delivery_source_response_matches(&response, &view));
    view.versions = &version; version.directory = "changed";
    assert(!delivery_source_response_matches(&response, &view));
    version = *snapshot.versions; version.next = (DeliverySnapshotVersion *)snapshot.versions;
    assert(!delivery_source_response_matches(&response, &view));
    version.next = NULL; version.configuration = snapshot.entries[0].text;
    assert(!delivery_source_response_matches(&response, &view));
    version.configuration = NULL;
    assert(!delivery_source_response_matches(&response, &view)); finish();
}
/** Exercise exact set admission and each independent refusal boundary. */
void run(void) {
    it_accepts_complete_exact_capture_and_empty_domains();
    it_refuses_changed_missing_duplicate_or_unobserved_sources();
    it_refuses_changed_or_ambiguous_companion_authority();
    it_refuses_changed_missing_duplicate_versions();
}
/** Keep generated evidence after failed assertions. */
int main(void) { run(); return 0; }
