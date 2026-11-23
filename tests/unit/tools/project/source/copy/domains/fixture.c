/** @file
 * @brief Own small real projects for same-identity promotion and root relocation.
 * @see [T275 contract](../../../../../../../docs/implementation/source-operations.md#t275-entry-review-and-execution)
 */
#include "../../../../../../../tools/project/source/copy/registry/retirement.h"
#include "../../../../../../../tools/project/source/transaction/guard/guard.h"
/** Test-only owners retire compiler/image borrowers before their original bytes. */
typedef struct {
    SourceArena arena;
    SourceRootCapture original;
    SourceObservedImage observed;
    SourceImageCapture final;
    SourceAnalysis analysis;
    SourceCompilerClient after;
    SourceCloneMap map;
    SourceCatalogImage image;
    ModuleRoot final_root;
    SourceTextEdits edits[4];
    SourceGraphPreparedText prepared[4];
    size_t edit_count;
    const char *parent, *project, *prefix, *destination, *opaque;
} DomainFixture;
/** Retain exact test keys without normalizing or guessing placement prefixes. */
static const char *domain_path(DomainFixture *fixture, const char *base, const char *suffix) {
    const char *path = source_arena_join(&fixture->arena, base, "/", suffix); assert(path); return path;
}
/** A real isolated compiler child consumes only the caller's serialized view. */
static void domain_compile(SourceCompilerClient *client, const DeliverySourceSnapshotView *view, const char *request_path) {
    FILE *request = fopen(request_path, "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, view) && fclose(request) == 0);
    SourceCompilerStatus status = source_compiler_client_run(client, "../../../../../../ama", request_path, view, NULL);
    if (status != SOURCE_COMPILER_OK && client->diagnostics) {
        fwrite(source_bytes(client->diagnostics), 1, source_length(client->diagnostics), stderr);
    }
    assert(status == SOURCE_COMPILER_OK && client->valid);
}
/** Prepare a dependency-only version and unrelated invalid bytes outside its
 * module roots. Relocation includes the empty destination in original analysis. */
static void domain_begin(DomainFixture *fixture, unsigned mode) {
    bool relocate = mode != 0, existing = mode == 1;
    setup_test(); memset(fixture, 0, sizeof(*fixture)); source_arena_init(&fixture->arena, sources.allocator);
    const char *base = mode == 2 ? "generated/domain-new-root" : relocate ? "generated/domain-relocation" : "generated/domain-promotion"; directory(base);
    SourcePhysicalPath parent; SourcePhysicalIssue issue;
    assert(source_physical_resolve(&fixture->arena, base, SOURCE_PHYSICAL_DIRECTORY, false, &parent, &issue) == SOURCE_PHYSICAL_OK);
    fixture->parent = parent.path; fixture->project = domain_path(fixture, parent.path, "original");
    const char *directories[] = {"", "dependencies", "dependencies/vendor", "dependencies/vendor/pkg",
        "dependencies/vendor/pkg/v1.0.0", "dependencies/vendor/pkg/v1.0.0/src", "dependencies/vendor/pkg/v1.0.0/tests",
        "dependencies/vendor/pkg/v1.0.0/tests/main", "dependencies/vendor/pkg/v1.0.0/tests/helper"};
    directory(fixture->project);
    for (size_t i = 1; i < sizeof(directories)/sizeof(*directories); i++) { directory(domain_path(fixture, fixture->project, directories[i])); }
    file(domain_path(fixture, fixture->project, "dependencies/vendor/pkg/v1.0.0/package.ini"), "");
    file(domain_path(fixture, fixture->project, "dependencies/vendor/pkg/v1.0.0/src/main"), "@!\nhelper&\n./helper\n");
    file(domain_path(fixture, fixture->project, "dependencies/vendor/pkg/v1.0.0/src/helper"), "@!\n");
    file(domain_path(fixture, fixture->project, "dependencies/vendor/pkg/v1.0.0/tests/main/check"), "@!\n");
    file(domain_path(fixture, fixture->project, "dependencies/vendor/pkg/v1.0.0/tests/helper/check"), "@!\n");
    fixture->opaque = domain_path(fixture, parent.path, "unrelated opaque"); file(fixture->opaque, "\xff not Ama");
    fixture->prefix = relocate ? fixture->project : domain_path(fixture, fixture->project, "dependencies");
    fixture->destination = domain_path(fixture, relocate ? parent.path : fixture->project, relocate ? "relocated" : "packages");
    if (existing) { directory(fixture->destination); }
    source_root_capture_init(&fixture->original, sources.allocator);
    ModuleRoot roots[] = {{fixture->project, NULL, 0, MODULE_ROOT_PROJECT}, {fixture->destination, NULL, 0, MODULE_ROOT_PROJECT}};
    assert(source_root_capture_build(&fixture->original, roots, existing ? 2 : 1) == SOURCE_CAPTURE_OK);
    source_analysis_init(&fixture->analysis, sources.allocator); source_compiler_client_init(&fixture->after, sources.allocator);
    FILE *request_file = fopen("generated/domain-before.json", "wb"); assert(request_file);
    assert(delivery_source_snapshot_write(request_file, &fixture->original.view) && fclose(request_file) == 0);
    assert(source_analysis_check(&fixture->analysis, &fixture->original.view, "../../../../../../ama",
        "generated/domain-before.json") == SOURCE_ANALYSIS_OK);
    source_observed_image_init(&fixture->observed, sources.allocator);
    SourceObservationRequest request = {fixture->opaque, SOURCE_OBSERVE_CONTENT, false};
    assert(source_observed_image_build(&fixture->observed, &fixture->original,
        (SourceImageBounds){&fixture->parent, 1}, &request, 1) == SOURCE_OBSERVATION_OK);
    source_image_capture_init(&fixture->final, sources.allocator); source_clone_map_init(&fixture->map, sources.allocator, &fixture->original);
    fixture->final_root = (ModuleRoot){relocate ? fixture->destination : fixture->project, NULL, 0, MODULE_ROOT_PROJECT};
}
/** Teardown never owns or deletes source paths; Make owns the leaf's fixtures. */
static void domain_end(DomainFixture *fixture) {
    source_compiler_client_destroy(&fixture->after); source_analysis_destroy(&fixture->analysis);
    source_image_capture_destroy(&fixture->final);
    for (size_t i = 0; i < fixture->edit_count; i++) { source_text_edits_destroy(&fixture->edits[i]); }
    source_clone_map_destroy(&fixture->map); source_observed_image_destroy(&fixture->observed);
    source_root_capture_destroy(&fixture->original); source_arena_destroy(&fixture->arena); destroy_test();
}
