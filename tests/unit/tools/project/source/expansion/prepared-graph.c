/** @file
 * @brief Compose exact resolved edits with real captured-input compilation and final graph/site checks.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 * This accepts explicit finite final-image discovery and compiler/graph handoff,
 * not complete operation selection/test closure or transaction publication.
 */
#include "../../../../../../tools/project/source/mapping/map.h"
#include "../../../../../../tools/project/source/observation/observation.h"
#include "../../../../../../tools/project/source/capture/image.h"
#include <stdint.h>
#include "../../../../../../tools/project/source/image/projection.h"
#include "../../../../../../tools/project/source/graph/sites.h"
#include "../../../../../../tools/project/source/operands/rooted.h"
/** Find one actual admitted definition, never reconstruct an edge with textual search. */
static const DeliverySourceDefinition *checked_definition(const DeliverySourceDefinition *definitions, const char *identity) {
    while (definitions && strcmp(definitions->identity, identity)) { definitions = definitions->next; }
    assert(definitions); return definitions;
}
/** Insert only the two declared new fixture mains into the recorded original
 * image. Dense labels preserve every observed original alias equivalence class;
 * each copy receives a distinct new class. Labels describe the virtual view, not
 * predicted OS inode values. Full operation material/transaction assembly is separate. */
static void prepared_image(SourceArena *arena, const SourceObservedImage *original,
    const ModuleSource *const sources[2], const SourceText *const copies[2], SourceCatalogImage *output) {
    size_t count = original->image.count;
    SourceCatalogImageEntry *rows = source_arena_allocate(arena, count * sizeof(*rows)); assert(rows);
    memcpy(rows, original->image.entries, count * sizeof(*rows));
    for (size_t i = 0; i < count; i++) {
        if (!rows[i].metadata_captured) { continue; }
        size_t representative = i;
        for (size_t j = 0; j < i; j++) {
            const SourceCatalogImageEntry *other = &original->image.entries[j];
            if (other->metadata_captured && other->metadata.st_dev == rows[i].metadata.st_dev &&
                other->metadata.st_ino == rows[i].metadata.st_ino) { representative = j; break; }
        }
        assert((uintmax_t)(ino_t)(representative + 1) == representative + 1);
        rows[i].metadata.st_dev = 0; rows[i].metadata.st_ino = (ino_t)(representative + 1);
    }
    SourceImageChange additions[2] = {0};
    for (size_t i = 0; i < 2; i++) {
        const SourceCatalogImageEntry *source = NULL;
        for (size_t j = 0; j < count; j++) {
            if (!strcmp(original->image.entries[j].path, source_path(sources[i]->text))) { source = &original->image.entries[j]; }
        }
        assert(source && source->metadata_captured && S_ISREG(source->metadata.st_mode));
        additions[i].kind = SOURCE_IMAGE_ADD; additions[i].entry = *source;
        additions[i].entry.path = source_path(copies[i]); additions[i].entry.content = copies[i];
        assert((uintmax_t)(ino_t)(count + i + 1) == count + i + 1);
        additions[i].entry.metadata.st_dev = 0; additions[i].entry.metadata.st_ino = (ino_t)(count + i + 1);
    }
    SourceImageIssue issue = {0};
    SourceCatalogImage base = {0}; SourceProjectionIssue projection;
    assert(source_catalog_image_prepare(&base, rows, count,
        original->image.roots, original->image.root_count, &issue) == SOURCE_IMAGE_OK);
    assert(source_image_project(arena, &base, additions, 2, output, &projection) == SOURCE_PROJECTION_OK);
}
/** Reparse actual edited copies, reject a valid wrong binding and reject an unresolved rewrite. */
static void it_reparses_prepared_cyclic_copies_and_checks_real_binding_and_site_provenance(void) {
    const char *identities[] = {"/vendor/pkg/v1.0.0/src/main", "/vendor/pkg/v1.0.0/src/other"};
    const char *copies[] = {"/vendor/pkg/v1.0.0/src/copied main", "/vendor/pkg/v1.0.0/src/copied other"};
    const char *basenames[] = {"copied main", "copied other"};
    for (unsigned choice = 0; choice < 3; choice++) {
        prepare();
        SourceArena scratch; source_arena_init(&scratch, sources.allocator);
        SourceManager prepared; source_manager_init(&prepared, sources.allocator);
        SourceCloneMap map; source_clone_map_init(&map, sources.allocator, &capture);
        SourceTextEdits edits[2]; SourceGraphPreparedText provenance[2] = {0};
        SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
        assert(source_observed_image_build(&observed, &capture, NULL, 0) == SOURCE_OBSERVATION_OK);
        SourceImageCapture final_capture; source_image_capture_init(&final_capture, sources.allocator);
        SourceCatalogImage image = {0}; const SourceText *copied_texts[2] = {0};
        const ModuleSource *originals[2] = {0}; const char *paths[2] = {0};
        for (size_t index = 0; index < 2; index++) {
            assert(module_catalog_find(&capture.catalog, identities[index], &originals[index]) == MODULE_OK);
            const char *directory = source_arena_join(&scratch, originals[index]->version->physical_path, "/", "src"); assert(directory);
            paths[index] = source_arena_join(&scratch, directory, "/", basenames[index]); assert(paths[index]);
            const SourceCloneMapping *row = NULL;
            assert(source_clone_map_add(&map, originals[index], paths[index], copies[index], &row) == SOURCE_CLONE_OK);
        }
        assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
        for (size_t index = 0; index < 2; index++) {
            const ModuleSource *source = originals[index];
            const DeliverySourceDefinition *definition = checked_definition(analysis.original.response.definitions, identities[index]);
            assert(definition->dependencies && !definition->dependencies->next);
            SourceSpan site = definition->dependencies->origin;
            assert(source_span_valid(site) && !strcmp(source_path(site.source), source_path(source->text)));
            assert(source_length(site.source) == source_length(source->text)
                && !memcmp(source_bytes(site.source), source_bytes(source->text), source_length(source->text)));
            source_text_edits_init(&edits[index], sources.allocator, site.source);
            if (index == 0 && choice) {
                const char *replacement = choice == 1 ? "./other" : "./missing";
                assert(source_text_edits_add(&edits[index], site, (const unsigned char *)replacement, strlen(replacement)) == SOURCE_EDITS_OK);
            } else {
                SourceOperandIssue issue = {0};
                assert(source_operand_rooted(&edits[index], &map, definition->dependencies, &issue) == SOURCE_OPERAND_OK);
            }
            const SourceText *text = NULL;
            assert(source_text_edits_apply(&edits[index], &prepared, paths[index], &text) == SOURCE_EDITS_OK);
            assert(access(paths[index], F_OK) != 0 && errno == ENOENT);
            copied_texts[index] = text;
            provenance[index] = (SourceGraphPreparedText){.next = index ? NULL : &provenance[1], .edits = &edits[index]};
        }
        prepared_image(&scratch, &observed, originals, copied_texts, &image);
        assert(source_image_capture_build(&final_capture, &image) == SOURCE_IMAGE_CAPTURE_OK && final_capture.valid);
        assert(final_capture.view.count == capture.view.count + 2);
        DeliverySourceSnapshotView view = final_capture.view;
        FILE *request = fopen("generated/final-request.json", "wb"); assert(request);
        assert(delivery_source_snapshot_write(request, &view) && fclose(request) == 0);
        SourceCompilerClient final; source_compiler_client_init(&final, sources.allocator);
        SourceCompilerStatus status = source_compiler_client_run(&final, "../../../../../../ama", "generated/final-request.json", &view, NULL);
        if (choice == 2) { assert(status == SOURCE_COMPILER_CHILD && !final.valid); }
        else {
            if (status != SOURCE_COMPILER_OK && final.diagnostics) {
                fwrite(source_bytes(final.diagnostics), 1, source_length(final.diagnostics), stderr);
            }
            assert(status == SOURCE_COMPILER_OK && final.valid);
            SourceSitesIssue issue = {0};
            assert(source_graph_sites(analysis.original.response.definitions, final.response.definitions, &map,
                NULL, 0, provenance, &issue) == (choice ? SOURCE_SITES_BINDINGS : SOURCE_SITES_OK));
            if (choice == 1) {
                assert(issue.binding_status == SOURCE_BINDINGS_EDGE);
                assert(issue.binding.original && !strcmp(issue.binding.original->identity, identities[0]));
                assert(issue.binding.expected && !strcmp(issue.binding.expected, copies[1]));
            }
        }
        source_compiler_client_destroy(&final);
        source_image_capture_destroy(&final_capture); source_observed_image_destroy(&observed);
        for (size_t index = 0; index < 2; index++) { source_text_edits_destroy(&edits[index]); }
        source_clone_map_destroy(&map); source_manager_destroy(&prepared); source_arena_destroy(&scratch); finish();
    }
}
