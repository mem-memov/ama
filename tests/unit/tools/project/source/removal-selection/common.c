/** @file
 * @brief Own leaf-local original compiler, raw tree and removal fixtures before each independent composition check.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#ifndef AMA_SOURCE_REMOVAL_SELECTION_TEST_COMMON
#define AMA_SOURCE_REMOVAL_SELECTION_TEST_COMMON
#include "../selection/common.c"
#include "../../../../../../tools/project/source/removal/selection/selection.h"
#include "delivery/source/snapshot/write.h"
static SourceAnalysis analysis;
static SourceRemovalSelection removal;
/** Independent pre-admission fixture profiles, never runtime deletion policies. */
typedef enum { MATERIAL_NONE, MATERIAL_TESTS, MATERIAL_OPAQUE, MATERIAL_MODULE_ASSET,
    MATERIAL_NATIVE_ASSET, MATERIAL_STANDARD } FixtureMaterial;
/** Add test material before the original fixture is frozen for compiler analysis. */
static void test_material(void) {
    directory("generated/project/packages/vendor/pkg/v1.0.0/tests/parent");
    directory("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child");
    file("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/check",
        "@!\nvalue&\n/vendor/pkg/v1.0.0/src/parent/child\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/helper", "@<\nvalue\n.T\n@!\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/data.bin", "explicit test data\n");
    file("generated/project/packages/vendor/pkg/v1.0.0/tests/outside",
        "@!\nvalue&\n/vendor/pkg/v1.0.0/src/parent/child\n");
}
/** Construct all physical fixtures before one original capture/analysis admission. */
static void begin_removal(FixtureMaterial tests, const char *operand) {
    if (tests == MATERIAL_STANDARD) { prepare_scope(true, true); }
    else { prepare(); }
    if (tests != MATERIAL_NONE && tests != MATERIAL_STANDARD) {
        test_material();
        if (tests == MATERIAL_OPAQUE) {
            assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/check") == 0);
            assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/helper") == 0);
            assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/outside") == 0);
        }
        if (tests == MATERIAL_MODULE_ASSET) {
            file("generated/project/packages/vendor/pkg/v1.0.0/src/remote", "@!\n");
            file("generated/project/packages/vendor/pkg/v1.0.0/src/parentish", "@!\nremote&\n./remote\n");
            file("generated/project/packages/vendor/pkg/v1.0.0/src/parentish.extra", "complete companion\n");
            directory("generated/project/packages/vendor/pkg/v1.0.0/src/parentish+");
            file("generated/project/packages/vendor/pkg/v1.0.0/src/parentish+/nested", "@!\n");
        }
        if (tests == MATERIAL_NATIVE_ASSET) {
            file("generated/project/packages/vendor/pkg/v1.0.0/src/native", "@C\n");
            file("generated/project/packages/vendor/pkg/v1.0.0/src/native.abi", "+ initialize\n- destroy\n");
            file("generated/project/packages/vendor/pkg/v1.0.0/src/native.c", "not executed C\n");
            file("generated/project/packages/vendor/pkg/v1.0.0/src/native.h", "not compiled header\n");
            file("generated/project/packages/vendor/pkg/v1.0.0/src/native.cmake", "message(FATAL_ERROR must_not_execute)\n");
            file("generated/project/packages/vendor/pkg/v1.0.0/src/native.ini", "; native scope\n");
            file("generated/project/packages/vendor/pkg/v1.0.0/README.md", "optional guidance\n");
        }
        source_root_capture_destroy(&capture); source_root_capture_init(&capture, sources.allocator);
        ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
        assert(source_root_capture_build(&capture, &root, 1) == SOURCE_CAPTURE_OK);
    }
    source_analysis_init(&analysis, sources.allocator);
    FILE *request = fopen("generated/request.json", "wb"); assert(request);
    assert(delivery_source_snapshot_write(request, &capture.view) && fclose(request) == 0);
    assert(source_analysis_check(&analysis, &capture.view, "../../../../../../ama", "generated/request.json") == SOURCE_ANALYSIS_OK);
    assert(source_tree_selection_read(&selection, &capture, operand) == SOURCE_TREE_OK);
    source_removal_selection_init(&removal, sources.allocator);
}
/** Retire review borrowers and remove optional fixture tests before the next independent case. */
static void end_removal(FixtureMaterial tests) {
    source_removal_selection_destroy(&removal); source_analysis_destroy(&analysis); finish();
    if (tests != MATERIAL_NONE && tests != MATERIAL_STANDARD) {
        if (tests != MATERIAL_OPAQUE) {
            assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/check") == 0);
            assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/helper") == 0);
            assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/outside") == 0);
        }
        assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child/data.bin") == 0);
        assert(rmdir("generated/project/packages/vendor/pkg/v1.0.0/tests/parent/child") == 0);
        assert(rmdir("generated/project/packages/vendor/pkg/v1.0.0/tests/parent") == 0);
        if (tests == MATERIAL_MODULE_ASSET) {
            assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/remote") == 0);
            assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/parentish.extra") == 0);
            assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/parentish+/nested") == 0);
            assert(rmdir("generated/project/packages/vendor/pkg/v1.0.0/src/parentish+") == 0);
            file("generated/project/packages/vendor/pkg/v1.0.0/src/parentish", "@!\n");
        }
        if (tests == MATERIAL_NATIVE_ASSET) {
            const char *paths[] = {"src/native", "src/native.abi", "src/native.c", "src/native.h",
                "src/native.cmake", "src/native.ini", "README.md"};
            char path[256];
            for (size_t index = 0; index < sizeof(paths) / sizeof(*paths); index++) {
                int length = snprintf(path, sizeof(path), "generated/project/packages/vendor/pkg/v1.0.0/%s", paths[index]);
                assert(length > 0 && (size_t)length < sizeof(path) && unlink(path) == 0);
            }
        }
    }
}
/** Query original checked handles, never copied declarations or inferred test names. */
static const DeliverySourceDefinition *definition(const char *identity) {
    for (const DeliverySourceDefinition *item = analysis.original.response.definitions; item; item = item->next) {
        if (!strcmp(item->identity, identity)) { return item; }
    }
    assert(false); return NULL;
}
/** Resolve exact original source-unit ownership for explicit caller input metadata. */
static const SourceCapturedUnit *unit(const char *identity) {
    for (const SourceCapturedUnit *item = capture.units; item; item = item->next) {
        if (!strcmp(item->source->logical_path, identity)) { return item; }
    }
    assert(false); return NULL;
}
/** Capture a separately identified real asset outside the selected source/test trees. */
static void asset(AcquisitionUnitEntry *entry, const char *bytes) {
    const SourceCapturedUnit *owner = unit("/vendor/pkg/v1.0.0/src/parent/child");
    const char *parent = source_arena_join(&selection.arena, owner->source->version->physical_path, "/", "src");
    const char *path = source_arena_join(&selection.arena, parent, "/", "shared.data"); assert(parent && path);
    file(path, bytes); *entry = (AcquisitionUnitEntry){0};
    assert(delivery_file_capture(&selection.arena, &selection.files, parent, "shared.data", &entry->file) == SOURCE_OK);
    assert(entry->file.state == DELIVERY_FILE_PRESENT);
}

#endif
