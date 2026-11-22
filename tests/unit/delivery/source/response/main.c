/** @file
 * @brief Verify exact independent response observations, graph ownership and malformed/failure refusal.
 * @see [T273 contract](../../../../../docs/implementation/source-protocol.md)
 */
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../../source/common.c"
#include "delivery/source/response/response.h"
static DeliverySourceResponse response;
static SourceManager request;
static const SourceText *input;
/** Encode controlled models independently of the production response reader. */
static void generate(void) {
    pid_t child = fork(); assert(child >= 0);
    if (!child) { execlp("python3", "python3", "generate.py", (char *)NULL); _exit(127); }
    int status;
    assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Original JSON has a separate lifetime from accepted capture/graph data. */
static void prepare(const char *name) {
    setup_test(); source_manager_init(&request, sources.allocator);
    delivery_source_response_init(&response, sources.allocator);
    char path[256]; int length = snprintf(path, sizeof(path), "generated/%s", name);
    assert(length > 0 && (size_t)length < sizeof(path));
    SourceStatus loaded = source_manager_load(&request, path, &input); assert(loaded == SOURCE_OK);
}
/** Retire graph before captured bytes, keeping diagnostics alive through failure. */
static void finish(void) {
    delivery_source_response_destroy(&response); source_manager_destroy(&request); destroy_test();
}
/** Query one known exact definition after decoder uniqueness/order checks. */
static const DeliverySourceDefinition *definition(const char *identity) {
    for (const DeliverySourceDefinition *item = response.definitions; item; item = item->next) {
        if (!strcmp(item->identity, identity)) { return item; }
    }
    assert(false); return NULL;
}
/** Original owner source bytes, parameterization and native/draft observation
 * state survive JSON/syntax teardown without a frontend pointer. */
static void it_retains_owned_sources_native_states_and_original_graph_sites(void) {
    generate(); prepare("response.json");
    SourceJsonStatus status = delivery_source_response_read(&response, input);
    assert(status == SOURCE_JSON_OK && response.valid && response.versions);
    assert(!strcmp(response.versions->identity, "/vendor/pkg/v1.0.0"));
    assert(definition("/vendor/pkg/v1.0.0/src/helper")->kind == DELIVERY_SOURCE_PARAMETERIZED);
    const DeliverySourceDefinition *main = definition("/vendor/pkg/v1.0.0/src/main");
    assert(main->dependencies && !main->dependencies->next && !strcmp(main->dependencies->target, "//native"));
    assert(!main->dependencies->implicit && !main->dependencies->member);
    SourceSpan site = main->dependencies->origin;
    assert(source_span_valid(site) && !strcmp(source_path(site.source), "not-published/\xffmain"));
    assert(site.end - site.start == strlen("//native") && !memcmp(source_bytes(site.source) + site.start, "//native", strlen("//native")));
    bool native = false, draft = false;
    for (const DeliveryResponseSource *source = response.sources; source; source = source->next) {
        if (!strcmp(source->identity, "//native")) {
            native = source->native_captured && source->native_abi && !strcmp(source->native_body, "not-published/native.c");
        }
        if (!strcmp(source->identity, "/vendor/pkg/v1.0.0/src/draft")) {
            draft = !source->configuration_captured && !source->configuration && !source->native_captured;
        }
    }
    assert(native && draft);
    source_manager_destroy(&request);
    assert(!strcmp(main->dependencies->target, "//native") && source_bytes(site.source)[0] == '@');
    finish();
}
/** Active graph cycles need no recursive expansion; normalized constant members
 * are retained even when a whole written operand is an alias. */
static void it_preserves_finite_cycles_and_normalized_constant_members(void) {
    generate(); prepare("cycle-response.json");
    SourceJsonStatus status = delivery_source_response_read(&response, input);
    assert(status == SOURCE_JSON_OK);
    const DeliverySourceDefinition *main = definition("/vendor/pkg/v1.0.0/src/main");
    const DeliverySourceDefinition *helper = definition("/vendor/pkg/v1.0.0/src/helper");
    assert(!strcmp(main->dependencies->target, helper->identity) && !strcmp(helper->dependencies->target, main->identity));
    finish();
    prepare("constant-response.json");
    status = delivery_source_response_read(&response, input); assert(status == SOURCE_JSON_OK);
    main = definition("/vendor/pkg/v1.0.0/src/main");
    assert(!strcmp(main->dependencies->target, "//palette") && !strcmp(main->dependencies->member, "values"));
    SourceSpan site = main->dependencies->origin;
    assert(site.end - site.start == strlen("~chosen") && !memcmp(source_bytes(site.source) + site.start, "~chosen", strlen("~chosen")));
    assert(definition("//palette")->kind == DELIVERY_SOURCE_CONSTANTS);
    finish();
}
/** Every malformed observation/identity/category/link/site case is refused as
 * an invalid reply, never repaired or converted into an accepted graph. */
static void it_refuses_malformed_response_observations_graphs_and_spans(void) {
    generate(); FILE *manifest = fopen("generated/bad-responses.txt", "r"); assert(manifest);
    char name[128]; size_t count = 0;
    while (fscanf(manifest, "%127s", name) == 1) {
        prepare(name);
        SourceJsonStatus status = delivery_source_response_read(&response, input);
        assert(status == SOURCE_JSON_INVALID && response.attempted && !response.valid);
        assert(response.issue.message && response.issue.origin.source == input && source_span_valid(response.issue.origin));
        finish(); count++;
    }
    assert(!ferror(manifest) && fclose(manifest) == 0 && count == 36);
}
/** Every syntax/capture/observation/graph allocation can fail with no accepted
 * partial response; all complete and failed owners are reclaimed. */
static void it_retires_every_failed_response_allocation(void) {
    generate(); size_t allocations = 0;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        prepare("response.json"); size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceJsonStatus status = delivery_source_response_read(&response, input);
        if (!failure) { assert(status == SOURCE_JSON_OK); allocations = allocation_count - before; assert(allocations); }
        else { assert(status == SOURCE_JSON_OUT_OF_MEMORY && response.attempted && !response.valid && response.issue.message); }
        failure_at = 0; finish();
    }
}
/** Empty replies create no source or graph; missing/reused owners cannot
 * overwrite previously accepted data. */
static void it_accepts_empty_responses_and_refuses_invalid_or_reused_owners(void) {
    generate(); prepare("empty-response.json");
    assert(delivery_source_response_read(NULL, input) == SOURCE_JSON_INVALID);
    assert(delivery_source_response_read(&response, NULL) == SOURCE_JSON_INVALID && !response.attempted);
    SourceJsonStatus status = delivery_source_response_read(&response, input);
    assert(status == SOURCE_JSON_OK && response.valid && !response.sources && !response.versions && !response.definitions);
    assert(delivery_source_response_read(&response, input) == SOURCE_JSON_INVALID && response.valid);
    finish();
}
/** Execute exact data, cycle/member, malformed and allocation contracts. */
void run(void) {
    it_retains_owned_sources_native_states_and_original_graph_sites();
    it_preserves_finite_cycles_and_normalized_constant_members();
    it_refuses_malformed_response_observations_graphs_and_spans();
    it_retires_every_failed_response_allocation();
    it_accepts_empty_responses_and_refuses_invalid_or_reused_owners();
}
/** Return only after the explicit counted assertion suite finishes. */
int main(void) { run(); return 0; }
