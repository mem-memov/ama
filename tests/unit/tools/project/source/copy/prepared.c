/** @file
 * @brief Compose actual copy main placement, parsed edits, simultaneous material and final compiler/site proof.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 * This fixture declares every added directory/main explicitly; it is not the
 * general opaque/context/configuration material planner or transaction runner.
 */
#include "../../../../../../tools/project/source/observation/observation.h"
#include "../../../../../../tools/project/source/capture/image.h"
#include "../../../../../../tools/project/source/image/projection.h"
#include "../../../../../../tools/project/source/operands/rooted.h"
#include "../../../../../../tools/project/source/graph/sites.h"
#include <stdint.h>
#include "../../../../../../tools/project/source/copy/members/units.h"
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
/** Original cyclic modules and their concrete tests are placed into a new actual
 * package layout in an immutable image, edited by spans and independently rechecked. */
static void it_joins_actual_copy_placement_and_projected_version_with_final_compiler_sites(void) {
    prepare(); select_main(); source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
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
    assert(source_copy_map_unit_members(&files, &copy, &map, &units) == SOURCE_COPY_UNITS_OK);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    size_t mapped = 0;
    for (const SourceFileMapping *file = files.first; file; file = file->next) { mapped++; }
    assert(mapped == 4);
    SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
    assert(source_observed_image_build(&observed, &capture, NULL, 0) == SOURCE_OBSERVATION_OK);
    const char *directories[] = {"packages/target", "packages/target/new", "packages/target/new/v3.0.0",
        "packages/target/new/v3.0.0/src", "packages/target/new/v3.0.0/tests",
        "packages/target/new/v3.0.0/tests/renamed", "packages/target/new/v3.0.0/tests/other"};
    SourceImageChange additions[12] = {0}; size_t used = 0;
    for (size_t i = 0; i < sizeof(directories) / sizeof(*directories); i++) {
        const char *path = source_arena_join(&arena, capture.roots[0].path, "/", directories[i]); assert(path);
        additions[used] = (SourceImageChange){SOURCE_IMAGE_ADD, {.path = path,
            .metadata = fresh_class(&observed.image, additions, used, true), .metadata_captured = true, .children_complete = true}}; used++;
    }
    SourceManager prepared; source_manager_init(&prepared, sources.allocator);
    const char *configuration_path = source_arena_join(&arena, version_path, "/", "package.ini"); assert(configuration_path);
    const SourceText *configuration = NULL;
    assert(source_manager_add(&prepared, configuration_path, (const unsigned char *)"", 0, &configuration) == SOURCE_OK);
    additions[used] = (SourceImageChange){SOURCE_IMAGE_ADD, {.path = configuration_path, .content = configuration,
        .metadata = fresh_class(&observed.image, additions, used, false), .metadata_captured = true}}; used++;
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
        additions[used] = (SourceImageChange){SOURCE_IMAGE_ADD, {.path = row->path, .content = text,
            .metadata = fresh_class(&observed.image, additions, used, false), .metadata_captured = true}}; used++;
        provenance[i] = (SourceGraphPreparedText){i == 3 ? NULL : &provenance[i + 1], &edits[i]};
    }
    assert(used == 12 && access(version_path, F_OK) != 0 && errno == ENOENT);
    SourceCatalogImage image = {0}; SourceProjectionIssue projection;
    assert(source_image_project(&arena, &observed.image, additions, used, &image, &projection) == SOURCE_PROJECTION_OK);
    SourceImageCapture final_capture; source_image_capture_init(&final_capture, sources.allocator);
    assert(source_image_capture_build(&final_capture, &image) == SOURCE_IMAGE_CAPTURE_OK);
    assert(final_capture.view.count == capture.view.count + 4);
    FILE *request = fopen("generated/final-copy.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &final_capture.view) && fclose(request) == 0);
    SourceCompilerClient final; source_compiler_client_init(&final, sources.allocator);
    assert(source_compiler_client_run(&final, "../../../../../../ama", "generated/final-copy.json", &final_capture.view, NULL) == SOURCE_COMPILER_OK);
    SourceSitesIssue sites;
    assert(source_graph_sites(analysis.original.response.definitions, final.response.definitions, &map, NULL, 0, provenance, &sites) == SOURCE_SITES_OK);
    source_compiler_client_destroy(&final); source_image_capture_destroy(&final_capture);
    for (size_t i = 0; i < 4; i++) { source_text_edits_destroy(&edits[i]); }
    source_manager_destroy(&prepared); source_observed_image_destroy(&observed); source_file_map_destroy(&files); source_clone_map_destroy(&map);
    source_copy_destinations_destroy(&destinations); source_clone_map_destroy(&explicit_map); source_arena_destroy(&arena); finish_copy();
}
