/** @file
 * @brief Compose actual copy main placement, parsed edits, simultaneous material and final compiler/site proof.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 * Production assembly derives mapped rows and required ancestor scaffolding.
 * Configuration eligibility, safe removal and publication remain separate gates.
 */
#include "../../../../../../tools/project/source/observation/observation.h"
#include "../../../../../../tools/project/source/capture/image.h"
#include "../../../../../../tools/project/source/image/projection.h"
#include "../../../../../../tools/project/source/operands/rooted.h"
#include "../../../../../../tools/project/source/graph/sites.h"
#include <stdint.h>
#include "../../../../../../tools/project/source/copy/image/image.h"
#include "../../../../../../tools/project/source/copy/proof/proof.h"
#include "../../../../../../tools/project/source/copy/configuration/version.h"
#include "../../../../../../tools/project/source/copy/members/units.h"
#include "../../../../../../tools/project/source/copy/tests/tests.h"
/** Choose the least unused synthetic (device 0,inode) class. At most the finite
 * original-plus-added row count can block candidates; labels model distinct new
 * entries and do not predict host inodes. All real original classes are retained. */
static struct stat fresh_class(const SourceCatalogImage *original, const SourceImageChange *added,
    size_t count, bool directory_entry) {
    mode_t mode = 0; bool found_mode = false;
    for (size_t i = 0; i < original->count; i++) {
        const SourceCatalogImageEntry *row = &original->entries[i];
        if (row->metadata_captured && (directory_entry ? S_ISDIR(row->metadata.st_mode) : S_ISREG(row->metadata.st_mode))) {
            mode = row->metadata.st_mode; found_mode = true; break;
        }
    }
    assert(found_mode); uintmax_t label = 1;
    for (;;) {
        assert((uintmax_t)(ino_t)label == label);
        bool occupied = false;
        for (size_t i = 0; i < original->count; i++) {
            const SourceCatalogImageEntry *row = &original->entries[i];
            occupied |= row->metadata_captured && row->metadata.st_dev == 0 && (uintmax_t)row->metadata.st_ino == label;
        }
        for (size_t i = 0; i < count; i++) { occupied |= (uintmax_t)added[i].entry.metadata.st_ino == label; }
        if (!occupied) { return (struct stat){.st_dev = 0, .st_ino = (ino_t)label, .st_mode = mode}; }
        assert(label < UINTMAX_MAX); label++;
    }
}
/** Resolve actual exported definition ownership independently of linked order. */
static const DeliverySourceDefinition *copy_definition(const char *identity) {
    for (const DeliverySourceDefinition *definition = analysis.original.response.definitions; definition; definition = definition->next) {
        if (!strcmp(definition->identity, identity)) { return definition; }
    }
    assert(false); return NULL;
}
#include "proof-cases.c"
#include "plan-cases.c"
/** Original cyclic modules and their concrete tests are placed into a new actual
 * package layout in an immutable image, edited by spans and independently rechecked. */
static void it_joins_actual_copy_placement_and_projected_version_with_final_compiler_sites(void) {
    setup_test(); fixtures();
    file("generated/project/packages/vendor/pkg/v1.0.0/src/main.payload", "opaque companion\n");
    directory("generated/project/packages/vendor/pkg/v1.0.0/src/main+");
    directory("generated/project/packages/vendor/pkg/v1.0.0/src/main+/empty");
    const unsigned char opaque[] = {'x', 0, 255};
    FILE *asset = fopen("generated/project/packages/vendor/pkg/v1.0.0/src/main+/data.bin", "wb"); assert(asset);
    assert(fwrite(opaque, 1, sizeof(opaque), asset) == sizeof(opaque) && fclose(asset) == 0);
    capture_fixtures(); select_main(); source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
    SourceArena arena; source_arena_init(&arena, sources.allocator);
    const char *version_path = source_arena_join(&arena, capture.roots[0].path, "/", "packages/target/new/v3.0.0"); assert(version_path);
    const char *main_path = source_arena_join(&arena, version_path, "/", "src/renamed"); assert(main_path);
    SourceCloneMap explicit_map; source_clone_map_init(&explicit_map, sources.allocator, &capture);
    const SourceCloneMapping *row = NULL;
    assert(source_clone_map_add(&explicit_map, tree.units[0]->source, main_path, "/target/new/v3.0.0/src/renamed", &row) == SOURCE_CLONE_OK);
    SourceCopyDestinations destinations; source_copy_destinations_init(&destinations, sources.allocator);
    assert(source_copy_destinations_build(&destinations, &copy, &explicit_map) == SOURCE_COPY_DESTINATIONS_OK && destinations.count == 4);
    const char *names[4];
    for (size_t i = 0; i < 4; i++) { names[i] = destinations.entries[i].identity; }
    SourceVersionPlacement version = {"/target/new/v3.0.0", version_path}; SourceCopyMainsIssue placement;
    SourceCloneMap map; source_clone_map_init(&map, sources.allocator, &capture);
    assert(source_copy_place_version_mains(&map, &destinations, &version, names, 4, &placement) == SOURCE_COPY_MAINS_OK);
    assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
    SourceFileMap files; source_file_map_init(&files, sources.allocator); SourceCopyUnitsIssue units;
    assert(source_copy_map_unit_members(&files, &copy, &map, NULL, &units) == SOURCE_COPY_UNITS_OK);
    SourceCopyTestsIssue test_material;
    assert(source_copy_map_test_members(&files, &copy, &map, &version, 1, &test_material) == SOURCE_COPY_TESTS_OK);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    size_t mapped = 0;
    for (const SourceFileMapping *file = files.first; file; file = file->next) { mapped++; }
    assert(mapped == 10);
    SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
    SourceObservationRequest requests[10]; size_t requested = 0;
    for (const SourceFileMapping *file = files.first; file; file = file->next) {
        assert(requested < 10);
        requests[requested++] = (SourceObservationRequest){file->source,
            file->original->directory ? SOURCE_OBSERVE_CHILDREN : SOURCE_OBSERVE_CONTENT, false};
    }
    assert(requested == mapped);
    assert(source_observed_image_build(&observed, &capture, (SourceImageBounds){0}, requests, requested) == SOURCE_OBSERVATION_OK);
    SourceManager prepared; source_manager_init(&prepared, sources.allocator);
    const SourceCatalogImageEntry *configuration_mode = NULL;
    const char *original_configuration_path = source_path(tree.units[0]->source->version->configuration);
    for (size_t i = 0; i < observed.image.count; i++) {
        if (!strcmp(observed.image.entries[i].path, original_configuration_path)) { configuration_mode = &observed.image.entries[i]; }
    }
    SourceVersionInitialization initialized; SourceVersionInitializationIssue initialization_issue;
    assert(source_version_initialize(&arena, &prepared, &observed, &version, &files, configuration_mode, NULL, 0,
        &initialized, &initialization_issue) == SOURCE_VERSION_INITIALIZATION_OK);
    assert(initialized.count == 1 && !source_length(initialized.configuration));
    SourceTextEdits edits[4]; SourceGraphPreparedText provenance[4];
    for (size_t i = 0; i < 4; i++) {
        row = map.first;
        while (row && row->original != destinations.entries[i].source) { row = row->next; } assert(row);
        const SourceFileMapping *file = files.first;
        while (file && strcmp(file->source, source_path(row->original->text))) { file = file->next; }
        assert(file && !strcmp(file->destination, row->path) && !file->original->directory);
        const DeliverySourceDefinition *definition = copy_definition(row->original->logical_path);
        const SourceText *original = definition->dependencies ? definition->dependencies->origin.source : row->original->text;
        source_text_edits_init(&edits[i], sources.allocator, original);
        for (const DeliverySourceDependency *edge = definition->dependencies; edge; edge = edge->next) {
            SourceOperandIssue issue;
            assert(source_operand_rooted(&edits[i], &map, edge, &issue) == SOURCE_OPERAND_OK);
        }
        const SourceText *text = NULL;
        assert(source_text_edits_apply(&edits[i], &prepared, row->path, &text) == SOURCE_EDITS_OK);
        provenance[i] = (SourceGraphPreparedText){i == 3 ? NULL : &provenance[i + 1], &edits[i]};
    }
    const SourceCatalogImage *image = NULL; SourceCopyImageIssue image_issue;
    SourceCopyImageStatus image_status = source_copy_image_prepare(&arena, &observed.image, &files, provenance, initialized.changes, initialized.count, observed.image.roots, observed.image.root_count,
        &image, &image_issue);
    if (image_status != SOURCE_COPY_IMAGE_OK) { fprintf(stderr, "image assembly: status=%d path=%s material=%d projection=%d\n",
        image_status, image_issue.path ? image_issue.path : "(none)", image_issue.material_status, image_issue.projection_status); }
    assert(image_status == SOURCE_COPY_IMAGE_OK);
    assert(image && access(version_path, F_OK) != 0 && errno == ENOENT);
    const char *opaque_path = source_arena_join(&arena, version_path, "/", "src/renamed+/data.bin"); assert(opaque_path);
    const char *empty_path = source_arena_join(&arena, version_path, "/", "src/renamed+/empty"); assert(empty_path);
    bool found_opaque = false, found_empty = false;
    for (size_t i = 0; i < image->count; i++) {
        const SourceCatalogImageEntry *entry = &image->entries[i];
        if (!strcmp(entry->path, opaque_path)) { found_opaque = true; assert(entry->content && source_length(entry->content) == sizeof(opaque)
            && !memcmp(source_bytes(entry->content), opaque, sizeof(opaque))); }
        if (!strcmp(entry->path, empty_path)) { found_empty = true; assert(S_ISDIR(entry->metadata.st_mode) && entry->children_complete && !entry->content); }
    }
    assert(found_opaque && found_empty);
    SourceImageCapture final_capture; source_image_capture_init(&final_capture, sources.allocator);
    assert(source_image_capture_build(&final_capture, image) == SOURCE_IMAGE_CAPTURE_OK);
    assert(final_capture.view.count == capture.view.count + 4);
    FILE *request = fopen("generated/final-copy.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &final_capture.view) && fclose(request) == 0);
    SourceCompilerClient final; source_compiler_client_init(&final, sources.allocator);
    assert(source_compiler_client_run(&final, "../../../../../../ama", "generated/final-copy.json", &final_capture.view, NULL) == SOURCE_COMPILER_OK);
    SourceCopyContextOmissions contexts;
    source_copy_context_omissions_init(&contexts, sources.allocator, &copy, &map);
    assert(source_copy_context_omissions_seal(&contexts) == SOURCE_COPY_CONTEXT_OK);
    SourceCopyProofIssue proof;
    assert(source_copy_prove(&arena, &analysis, &map, &files, provenance, &contexts, &final_capture, &final, NULL, 0, &proof) == SOURCE_COPY_PROOF_OK);
    assert(!proof.path && !proof.unit);
    assert(source_copy_prove(&arena, &analysis, &map, &files, provenance, &contexts, &final_capture, &analysis.original, NULL, 0, &proof) == SOURCE_COPY_PROOF_INPUTS);
    size_t before_proof = allocation_count;
    failure_at = before_proof + 1;
    assert(source_copy_prove(&arena, &analysis, &map, &files, provenance, &contexts, &final_capture, &final, NULL, 0, &proof) == SOURCE_COPY_PROOF_MATERIAL);
    assert(proof.material_status == SOURCE_MATERIAL_MEMORY); failure_at = 0;
    assert(source_copy_prove(&arena, &analysis, &map, &files, provenance, &contexts, &final_capture, &final, NULL, 0, &proof) == SOURCE_COPY_PROOF_OK);
    check_copy_proof_refusals(&arena, &prepared, &map, &files, provenance, &contexts, &final_capture, &final, opaque_path);
    check_copy_plan(&arena, &observed, &map, &files, provenance, &contexts, &final_capture, &final, initialized.changes, initialized.count);
    source_copy_context_omissions_destroy(&contexts);
    source_compiler_client_destroy(&final); source_image_capture_destroy(&final_capture);
    for (size_t i = 0; i < 4; i++) { source_text_edits_destroy(&edits[i]); }
    source_manager_destroy(&prepared); source_observed_image_destroy(&observed); source_file_map_destroy(&files); source_clone_map_destroy(&map);
    source_copy_destinations_destroy(&destinations); source_clone_map_destroy(&explicit_map); source_arena_destroy(&arena); finish_copy();
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/main.payload") == 0);
    assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/main+/data.bin") == 0);
    assert(rmdir("generated/project/packages/vendor/pkg/v1.0.0/src/main+/empty") == 0);
    assert(rmdir("generated/project/packages/vendor/pkg/v1.0.0/src/main+") == 0);
}
