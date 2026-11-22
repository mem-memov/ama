/** @file
 * @brief Exercise complete configured local capture, opaque companion bytes and failure cleanup.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/capture/capture.h"
#include "../../../../../../tools/project/source/capture/internal.h"
#include "../../../../../../tools/project/source/compiler/client.h"
#include "delivery/source/snapshot/write.h"
static SourceRootCapture capture;
static const ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
    {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
/** Make fixed test-owned directories without forgiving unexpected filesystem errors. */
static void directory(const char *path) { assert(mkdir(path, 0700) == 0 || errno == EEXIST); }
/** Preserve arbitrary bytes rather than interpreting companion formats. */
static void file(const char *path, const unsigned char *bytes, size_t length) {
    FILE *output = fopen(path, "wb"); assert(output);
    assert(fwrite(bytes, 1, length, output) == length && fclose(output) == 0);
}
/** Write exact small text fixtures. */
static void text(const char *path, const char *content) { file(path, (const unsigned char *)content, strlen(content)); }
/** Stable fixture includes an ignored competing version copy and an empty selected version. */
static void fixtures(void) {
    const char *directories[] = {"generated/project", "generated/project/packages", "generated/project/packages/vendor",
        "generated/project/packages/vendor/pkg", "generated/project/packages/vendor/pkg/v1.0.0",
        "generated/project/packages/vendor/pkg/v1.0.0/src", "generated/project/packages/vendor/pkg/v1.0.0/tests",
        "generated/project/packages/vendor/pkg/v1.0.0/src/main+", "generated/project/packages/vendor/pkg/v2.0.0",
        "generated/project/packages/vendor/pkg/v2.0.0/src", "generated/project/packages/vendor/pkg/v2.0.0/tests",
        "generated/project/dependencies", "generated/project/dependencies/vendor", "generated/project/dependencies/vendor/pkg",
        "generated/project/dependencies/vendor/pkg/v1.0.0", "generated/project/dependencies/vendor/pkg/v1.0.0/src", "generated/standard"};
    for (size_t index = 0; index < sizeof(directories) / sizeof(*directories); index++) { directory(directories[index]); }
    text("generated/project/packages/vendor/pkg/v1.0.0/package.ini", "; selected\n");
    text("generated/project/packages/vendor/pkg/v2.0.0/package.ini", "; empty\n");
    text("generated/project/dependencies/vendor/pkg/v1.0.0/package.ini", "; ignored\n");
    text("generated/project/packages/vendor/pkg/v1.0.0/src/main", "@!\nlink&\n//native\n");
    text("generated/project/packages/vendor/pkg/v1.0.0/src/main.ini", "; local\n");
    text("generated/project/packages/vendor/pkg/v1.0.0/src/main+/child", "@!\n");
    text("generated/project/packages/vendor/pkg/v1.0.0/src/helper", "@<\nvalue\n.T\n@!\n");
    text("generated/project/packages/vendor/pkg/v1.0.0/src/draft", "unfinished !\n");
    static const unsigned char binary[] = {255, 0, 13, 10};
    file("generated/project/packages/vendor/pkg/v1.0.0/src/draft.abi", binary, sizeof(binary));
    file("generated/project/packages/vendor/pkg/v1.0.0/src/main.extra", binary, sizeof(binary));
    file("generated/project/dependencies/vendor/pkg/v1.0.0/src/ignored", binary, sizeof(binary));
    text("generated/standard/native", "@C\n");
    text("generated/standard/native.abi", "+ initialize\n- destroy\n");
    text("generated/standard/native.c", "not executed C\n");
    text("generated/standard/native.h", "not compiled header\n");
    text("generated/standard/native.cmake", "message(FATAL_ERROR must_not_execute)\n");
}
/** Start explicit allocation/capture owners without source discovery side effects. */
static void prepare(void) { setup_test(); source_root_capture_init(&capture, sources.allocator); }
/** Count every source/metadata allocation after partial or complete capture. */
static void finish(void) { source_root_capture_destroy(&capture); destroy_test(); }
/** Find one exact independently expected captured row. */
static const ModuleSnapshotEntry *entry(const char *identity) {
    for (size_t index = 0; index < capture.view.count; index++) {
        if (!strcmp(capture.view.entries[index].logical_path, identity)) { return &capture.view.entries[index]; }
    }
    assert(false); return NULL;
}
/** Whole-version precedence, empty versions, unused templates and drafts remain distinct. */
static void it_captures_every_selected_source_and_opaque_companion_without_execution(void) {
    prepare(); assert(source_root_capture_build(&capture, roots, 2) == SOURCE_CAPTURE_OK && capture.valid);
    assert(capture.view.count == 5 && !capture.failed_path);
    assert(entry("/vendor/pkg/v1.0.0/src/main/child"));
    assert(entry("/vendor/pkg/v1.0.0/src/helper"));
    assert(entry("/vendor/pkg/v1.0.0/src/main")->configuration);
    size_t versions = 0;
    for (const DeliverySnapshotVersion *version = capture.view.versions; version; version = version->next) { versions++; }
    assert(versions == 2);
    bool draft = false, native = false, extra = false;
    for (const DeliverySourceCompanions *row = capture.view.companions; row; row = row->next) {
        if (!strcmp(row->identity, "/vendor/pkg/v1.0.0/src/draft")) {
            draft = row->abi && source_length(row->abi) == 4 && source_bytes(row->abi)[0] == 255 && !row->body;
        }
        if (!strcmp(row->identity, "//native")) { native = row->body && row->header && row->abi; }
    }
    for (const SourceCapturedUnit *unit = capture.units; unit; unit = unit->next) {
        for (const AcquisitionUnitEntry *raw = unit->files; raw; raw = raw->next) {
            if (strstr(raw->file.path, "main.extra")) { extra = source_length(raw->file.content) == 4; }
        }
    }
    assert(draft && native && extra);
    assert(source_root_capture_build(&capture, roots, 2) == SOURCE_CAPTURE_INVALID && capture.valid); finish();
}
/** Configured namespace kind is retained while relative keys/exclusions become independently owned. */
static void it_owns_normalized_root_keys_and_exclusions_without_namespace_inference(void) {
    prepare(); char project[] = "generated/project", standard[] = "generated/standard";
    char exclusion[] = "packages/vendor/pkg/v1.0.0/src/helper"; const char *excluded[] = {exclusion};
    ModuleRoot supplied[] = {{project, excluded, 1, MODULE_ROOT_PROJECT}, {standard, NULL, 0, MODULE_ROOT_STANDARD}};
    assert(source_root_capture_build(&capture, supplied, 2) == SOURCE_CAPTURE_OK && capture.view.count == 4);
    project[0] = '!'; standard[0] = '!'; exclusion[0] = '!';
    assert(capture.root_count == 2 && capture.roots[0].path[0] == '/' && capture.roots[1].path[0] == '/');
    assert(capture.roots[0].kind == MODULE_ROOT_PROJECT && capture.roots[1].kind == MODULE_ROOT_STANDARD);
    assert(!strcmp(capture.roots[0].excluded[0], "packages/vendor/pkg/v1.0.0/src/helper"));
    const ModuleSource *missing = NULL;
    assert(module_catalog_find(&capture.catalog, "/vendor/pkg/v1.0.0/src/helper", &missing) == MODULE_MISSING); finish();
    prepare(); ModuleRoot invalid = {"generated/absent", NULL, 0, MODULE_ROOT_PROJECT};
    assert(source_root_capture_build(&capture, &invalid, 1) == SOURCE_CAPTURE_ROOT);
    assert(!capture.valid && !capture.catalog.attempted && capture.root_issue.status == SOURCE_PHYSICAL_MISSING); finish();
    prepare(); invalid.path = "generated/project"; invalid.kind = (ModuleRootKind)0;
    assert(source_root_capture_build(&capture, &invalid, 1) == SOURCE_CAPTURE_INVALID && !capture.valid); finish();
    prepare(); invalid.kind = MODULE_ROOT_PROJECT; invalid.excluded_count = 1;
    assert(source_root_capture_build(&capture, &invalid, 1) == SOURCE_CAPTURE_INVALID && !capture.valid); finish();
    prepare(); const char *missing_exclusion[] = {NULL}; invalid.excluded = missing_exclusion;
    assert(source_root_capture_build(&capture, &invalid, 1) == SOURCE_CAPTURE_INVALID && !capture.valid); finish();
}
/** Serialize once, then make physical inputs unusable: actual analysis still consumes retained bytes. */
static void it_hands_immutable_captures_to_the_real_compiler_without_rereading_sources(void) {
    prepare(); assert(source_root_capture_build(&capture, roots, 2) == SOURCE_CAPTURE_OK);
    text("generated/standard/native.abi", "invalid now\n");
    text("generated/project/packages/vendor/pkg/v1.0.0/src/main", "invalid now\n");
    FILE *output = fopen("generated/request.json", "wb"); assert(output);
    assert(delivery_source_snapshot_write(output, &capture.view) && fclose(output) == 0);
    SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
    assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/request.json", &capture.view, NULL) == SOURCE_COMPILER_OK);
    assert(client.valid && client.response.definitions);
    source_compiler_client_destroy(&client);
    source_compiler_client_init(&client, sources.allocator);
    assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/request.json", &capture.view,
        "/vendor/pkg/v1.0.0/src/main") == SOURCE_COMPILER_OK && client.probe_state == DELIVERY_PROBE_EXECUTABLE);
    source_compiler_client_destroy(&client);
    source_compiler_client_init(&client, sources.allocator);
    assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/request.json", &capture.view,
        "/vendor/pkg/v1.0.0/src/helper") == SOURCE_COMPILER_OK && client.probe_state == DELIVERY_PROBE_OPEN);
    source_compiler_client_destroy(&client); finish(); fixtures();
}
/** Exact physical companion shape errors refuse without exposing a prepared view. */
static void it_refuses_nonregular_companions_and_catalog_aliases(void) {
    assert(unlink("generated/standard/native.h") == 0); directory("generated/standard/native.h");
    prepare(); assert(source_root_capture_build(&capture, roots, 2) == SOURCE_CAPTURE_FILES);
    assert(!capture.valid && !capture.view.entries && capture.failed_path); finish();
    assert(rmdir("generated/standard/native.h") == 0); fixtures();
    assert(symlink("native", "generated/standard/alias") == 0);
    prepare(); assert(source_root_capture_build(&capture, roots, 2) == SOURCE_CAPTURE_CATALOG && !capture.valid); finish();
    assert(unlink("generated/standard/alias") == 0);
}
/** Observe inconsistent main/configuration bytes through the real unit boundary, without races or retries. */
static void it_refuses_changed_catalog_main_and_configuration_observations(void) {
    prepare(); assert(module_catalog_build(&capture.catalog, roots, 2) == MODULE_OK);
    const ModuleSource *source = NULL;
    assert(module_catalog_find(&capture.catalog, "/vendor/pkg/v1.0.0/src/main", &source) == MODULE_OK);
    assert(module_catalog_load_configuration(&capture.catalog, source) == MODULE_OK);
    SourceCapturedUnit *unit = NULL;
    text(source_path(source->text), "changed main\n");
    assert(source_capture_unit(&capture, source, &unit) == SOURCE_CAPTURE_CHANGED && !unit);
    text(source_path(source->text), "@!\nlink&\n//native\n");
    const char *saved = "generated/project/packages/vendor/pkg/v1.0.0/src/main.saved";
    assert(rename(source_path(source->text), saved) == 0);
    text(source_path(source->text), "@!\nlink&\n//native\n");
    assert(source_capture_unit(&capture, source, &unit) == SOURCE_CAPTURE_CHANGED && !unit);
    assert(unlink(source_path(source->text)) == 0 && rename(saved, source_path(source->text)) == 0);
    text(source->configuration_path, "; changed ini\n");
    assert(source_capture_unit(&capture, source, &unit) == SOURCE_CAPTURE_CHANGED && !unit); finish(); fixtures();
}
/** Fail every allocation while requiring one-shot publication and complete teardown. */
static void it_retires_every_capture_allocation_failure_without_publishing_partial_rows(void) {
    const char *excluded[] = {"packages/vendor/pkg/v1.0.0/src/not-present"};
    ModuleRoot tested[] = {roots[0], roots[1]}; tested[0].excluded = excluded; tested[0].excluded_count = 1;
    prepare(); size_t before = allocation_count;
    assert(source_root_capture_build(&capture, tested, 2) == SOURCE_CAPTURE_OK);
    size_t count = allocation_count - before; finish(); assert(count);
    for (size_t ordinal = 1; ordinal <= count; ordinal++) {
        prepare(); failure_at = allocation_count + ordinal;
        assert(source_root_capture_build(&capture, tested, 2) != SOURCE_CAPTURE_OK && !capture.valid && !capture.view.entries);
        failure_at = 0; finish();
    }
}
/** Empty explicit standard roots do not synthesize a version or default source. */
static void it_captures_empty_domains_and_refuses_invalid_provider_arguments(void) {
    directory("generated/empty"); ModuleRoot empty = {"generated/empty", NULL, 0, MODULE_ROOT_STANDARD};
    prepare(); assert(source_root_capture_build(&capture, NULL, 1) == SOURCE_CAPTURE_INVALID && !capture.attempted);
    assert(source_root_capture_build(&capture, &empty, 1) == SOURCE_CAPTURE_OK && capture.valid && !capture.view.count && !capture.view.versions);
    finish();
}
/** Execute full physical capture, compiler composition, refusal and allocation partitions. */
void run(void) {
    fixtures();
    it_captures_every_selected_source_and_opaque_companion_without_execution();
    it_owns_normalized_root_keys_and_exclusions_without_namespace_inference();
    it_hands_immutable_captures_to_the_real_compiler_without_rereading_sources();
    it_refuses_nonregular_companions_and_catalog_aliases();
    it_refuses_changed_catalog_main_and_configuration_observations();
    it_retires_every_capture_allocation_failure_without_publishing_partial_rows();
    it_captures_empty_domains_and_refuses_invalid_provider_arguments();
}
/** Preserve generated evidence on failed assertions. */
int main(void) { run(); return 0; }
