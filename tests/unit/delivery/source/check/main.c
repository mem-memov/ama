/** @file
 * @brief Verify real entry-free checking of captured future paths, native metadata and failed inputs.
 * @see [T224 contract](../../../../../docs/implementation/source-operations.md)
 */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../../source/common.c"
#include "delivery/source/domain/domain.h"
#include "delivery/source/probe/snapshot.h"
static CompilerFrontend frontend;
static DeliverySourceSnapshot snapshot;
static SourceManager request;
static SourceArena exports;
static const DeliverySourceDefinition *definitions;
/** Reuse the independently encoded model in this leaf's own generated tree. */
static void generate(void) {
    pid_t child = fork(); assert(child >= 0);
    if (!child) { execlp("python3", "python3", "../snapshot/generate.py", (char *)NULL); _exit(127); }
    int status;
    assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Build healthy data owners before any independently injected frontend failure. */
static void prepare(const char *name) {
    setup_test(); compiler_frontend_init(&frontend, sources.allocator);
    source_manager_init(&request, sources.allocator);
    source_arena_init(&exports, sources.allocator);
    delivery_source_snapshot_init(&snapshot, sources.allocator); definitions = NULL;
    char path[256]; int length = snprintf(path, sizeof(path), "generated/%s", name);
    assert(length > 0 && (size_t)length < sizeof(path));
    const SourceText *input = NULL;
    SourceStatus loaded = source_manager_load(&request, path, &input); assert(loaded == SOURCE_OK);
    SourceJsonStatus parsed = delivery_source_snapshot_read(&snapshot, input); assert(parsed == SOURCE_JSON_OK);
}
/** Failed diagnostics may borrow input preparation; retain it through frontend
 * destruction while successful graph nodes retire before frontend source bytes. */
static void finish(void) {
    source_arena_destroy(&exports); compiler_frontend_destroy(&frontend);
    delivery_source_snapshot_destroy(&snapshot); source_manager_destroy(&request); destroy_test();
}
/** The same immutable request supports independent executable and genuinely open roots. */
static void it_probes_independent_captured_entries_with_exact_native_and_root_key_authority(void) {
    generate();
    const char *entries[] = {"/vendor/pkg/v1.0.0/src/main", "/vendor/pkg/v1.0.0/src/helper"};
    for (size_t index = 0; index < 2; index++) {
        prepare("base.json");
        DeliveryEntryState state = delivery_source_snapshot_entry(&frontend, &exports, &snapshot, entries[index]);
        assert(state == (index ? DELIVERY_ENTRY_OPEN : DELIVERY_ENTRY_EXECUTABLE));
        assert(!frontend.valid && !frontend.inputs.attempted && !frontend.inputs.sealed);
        assert(index || (frontend.plan_initialized && frontend.plan.valid));
        assert(access("not-published/native.c", F_OK) != 0); finish();
    }
    prepare("bad-native-abi.json");
    assert(delivery_source_snapshot_entry(&frontend, &exports, &snapshot, entries[0]) == DELIVERY_ENTRY_INVALID);
    assert(frontend.error.message && !frontend.valid); finish();
}
/** Future physical files do not exist. Canonical ordinary/standard identities,
 * package context, open definitions and the native interface still check exactly. */
static void it_checks_captured_future_paths_without_filesystem_or_native_execution(void) {
    generate(); prepare("base.json");
    assert(access("not-published", F_OK) != 0);
    bool checked = delivery_source_domain_snapshot(&frontend, &exports, &snapshot, &definitions);
    if (!checked) { fprintf(stderr, "Prepared snapshot refused: %s\n", frontend.error.message); }
    assert(checked && frontend.attempted && !frontend.valid && !frontend.plan_initialized && !frontend.project.entry);
    size_t count = 0; bool native = false, helper = false, main = false;
    for (const DeliverySourceDefinition *item = definitions; item; item = item->next) {
        count++;
        if (!strcmp(item->identity, "//native")) { native = item->kind == DELIVERY_SOURCE_CONCRETE; }
        if (!strcmp(item->identity, "/vendor/pkg/v1.0.0/src/helper")) { helper = item->kind == DELIVERY_SOURCE_PARAMETERIZED; }
        if (!strcmp(item->identity, "/vendor/pkg/v1.0.0/src/main")) {
            main = true; assert(item->dependencies && !strcmp(item->dependencies->target, "//native"));
        }
    }
    assert(count == 3 && native && helper && main && access("not-published", F_OK) != 0);
    delivery_source_snapshot_destroy(&snapshot); source_manager_destroy(&request);
    const ProjectModule *module = project_find(&frontend.project, "//native");
    assert(module && module->source->native_loaded && module->source->native_abi);
    assert(!strcmp(module->source->native_c, "not-published/native.c"));
    assert(frontend.project.catalog.versions && source_bytes(frontend.project.catalog.versions->configuration)[0] == ';');
    finish();
}
/** Syntactic/type, source encoding, native ABI encoding and physical alias
 * failures remain ordinary failures, with no first-match or reread fallback. */
static void it_refuses_invalid_prepared_code_abi_and_aliases_before_graph_publication(void) {
    generate();
    const char *names[] = {"bad-source.json", "bad-source-utf8.json", "bad-native-abi.json", "aliased-metadata.json"};
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        prepare(names[index]); DeliverySourceDefinition sentinel = {0}; definitions = &sentinel;
        bool checked = delivery_source_domain_snapshot(&frontend, &exports, &snapshot, &definitions);
        assert(!checked && frontend.attempted && definitions == &sentinel && frontend.error.message);
        assert(!frontend.valid && !frontend.plan_initialized);
        finish();
    }
}
/** Preparation, source/native classification, common semantic checking and
 * graph export failures all retain correct ownership and consume the attempt. */
static void it_retires_every_failed_captured_frontend_allocation(void) {
    generate(); size_t allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        prepare("base.json"); DeliverySourceDefinition sentinel = {0}; definitions = &sentinel;
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        bool checked = delivery_source_domain_snapshot(&frontend, &exports, &snapshot, &definitions);
        if (!failure) { assert(checked); allocations = allocation_count - before; assert(allocations); }
        else {
            assert(!checked && frontend.attempted && definitions == &sentinel && frontend.error.message);
        }
        failure_at = 0; finish();
    }
}
/** Empty input remains entry-free; invalid data/owner state and reuse cannot
 * replace an existing accepted graph or trigger a normal-filesystem fallback. */
static void it_accepts_empty_capture_and_refuses_invalid_or_reused_owners(void) {
    generate(); prepare("empty.json"); DeliverySourceDefinition sentinel = {0}; definitions = &sentinel;
    DeliverySourceSnapshot invalid = {0};
    assert(!delivery_source_domain_snapshot(&frontend, &exports, &invalid, &definitions));
    assert(!frontend.attempted && definitions == &sentinel);
    bool checked = delivery_source_domain_snapshot(&frontend, &exports, &snapshot, &definitions);
    assert(checked && !definitions && !frontend.project.entry);
    assert(!delivery_source_domain_snapshot(&frontend, &exports, &snapshot, &definitions));
    assert(!delivery_source_domain_snapshot(NULL, &exports, &snapshot, &definitions));
    assert(!delivery_source_domain_snapshot(&frontend, NULL, &snapshot, &definitions));
    assert(!delivery_source_domain_snapshot(&frontend, &exports, NULL, &definitions));
    assert(!delivery_source_domain_snapshot(&frontend, &exports, &snapshot, NULL));
    finish();
}
/** Explicitly execute real normal-check and source ownership handoffs. */
void run(void) {
    it_probes_independent_captured_entries_with_exact_native_and_root_key_authority();
    it_checks_captured_future_paths_without_filesystem_or_native_execution();
    it_refuses_invalid_prepared_code_abi_and_aliases_before_graph_publication();
    it_retires_every_failed_captured_frontend_allocation();
    it_accepts_empty_capture_and_refuses_invalid_or_reused_owners();
}
/** Preserve failed generated evidence and return only after all assertions. */
int main(void) { run(); return 0; }
