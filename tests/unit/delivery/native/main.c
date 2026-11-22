/** @file
 * @brief Check native-origin projection against actual concrete frontend plans.
 * @see [Native delivery contract](../../../../docs/implementation/build-launcher-contracts.md)
 */
#include "../../compiler/common.c"
#include "delivery/native/guidance.h"
#include <unistd.h>
static SourceArena retained;

/** Prepare repeated instances and unused native definitions; invalid native C
 * proves selection does not invoke a C compiler or execute a body.
 */
static void prepare_sources(const char *body) {
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", body);
    const char *names[] = {"a", "z"};
    for (size_t index = 0; index < 2; index++) {
        char path[160];
        snprintf(path, sizeof(path), "generated/project/packages/test/fixture/v1.0.0/src/%s", names[index]);
        file(path, "@C\n");
        snprintf(path, sizeof(path), "generated/project/packages/test/fixture/v1.0.0/src/%s.h", names[index]);
        file(path, "#error Native selection must not compile C\n");
        snprintf(path, sizeof(path), "generated/project/packages/test/fixture/v1.0.0/src/%s.c", names[index]);
        file(path, "#error Native selection must not execute C\n");
        snprintf(path, sizeof(path), "generated/project/packages/test/fixture/v1.0.0/src/%s.abi", names[index]);
        file(path, "+ initialize\n- destroy\n");
    }
    file("generated/project/packages/test/fixture/v1.0.0/src/unused", "@C\n");
}
/** Check already prepared physical inputs before borrowing plan/catalog views. */
static void begin_frontend(void) {
    setup_frontend();
    bool checked = check_project();
    assert(checked);
    source_arena_init(&retained, sources.allocator);
}
/** Compose source preparation and actual compiler admission for ordinary cases. */
static void prepare(const char *body) { prepare_sources(body); begin_frontend(); }
/** Retire relation storage before the catalog/plan it borrows. */
static void finish(void) { source_arena_destroy(&retained); destroy_frontend(); }

/** Repeated fields contribute one native module in full-name order. */
static void it_projects_only_actual_native_origins_in_identity_order(void) {
    prepare("@!\nlast\n/test/fixture/v1.0.0/src/z\nfirst\n/test/fixture/v1.0.0/src/a\n"
        "again\n/test/fixture/v1.0.0/src/z\n");
    const DeliveryNativeOrigin *origins = NULL; Diagnostic error = {0};
    bool selected = delivery_native_origins(&retained, &frontend, &origins, &error);
    assert(selected && origins != NULL && origins->next != NULL && origins->next->next == NULL);
    assert(strcmp(origins->source->logical_path, "/test/fixture/v1.0.0/src/a") == 0);
    assert(strcmp(origins->next->source->logical_path, "/test/fixture/v1.0.0/src/z") == 0);
    const ModuleSource *unused = NULL;
    ModuleStatus found = module_catalog_find(&frontend.project.catalog, "/test/fixture/v1.0.0/src/unused", &unused);
    assert(found == MODULE_OK && !unused->native_loaded);
    SourceArena other; source_arena_init(&other, sources.allocator);
    const DeliveryNativeOrigin *independent = NULL;
    selected = delivery_native_origins(&other, &frontend, &independent, &error);
    assert(selected && independent != origins && independent->source == origins->source);
    source_arena_destroy(&other); finish();
}
/** An empty projection is successful; rejected owners preserve caller output. */
static void it_distinguishes_empty_projection_from_invalid_frontend(void) {
    prepare("@!\n");
    DeliveryNativeOrigin sentinel = {0}; const DeliveryNativeOrigin *output = &sentinel;
    Diagnostic error = {0};
    bool selected = delivery_native_origins(&retained, &frontend, &output, &error);
    assert(selected && output == NULL);
    frontend.valid = false; output = &sentinel;
    selected = delivery_native_origins(&retained, &frontend, &output, &error);
    assert(!selected && output == &sentinel && error.code == DIAGNOSTIC_CHANGED_INPUT);
    frontend.valid = true; frontend.authoring = true;
    selected = delivery_native_origins(&retained, &frontend, &output, &error);
    assert(!selected && output == &sentinel);
    frontend.authoring = false; finish();
}
/** Every failed relation allocation leaves only destructible arena scratch. */
static void it_preserves_output_at_every_native_origin_allocation_failure(void) {
    prepare("@!\none\n/test/fixture/v1.0.0/src/a\ntwo\n/test/fixture/v1.0.0/src/z\n");
    const DeliveryNativeOrigin *output = NULL; Diagnostic error = {0};
    size_t before = allocation_count;
    bool selected = delivery_native_origins(&retained, &frontend, &output, &error);
    assert(selected);
    size_t total = allocation_count - before;
    for (size_t ordinal = 1; ordinal <= total; ordinal++) {
        SourceArena failing; source_arena_init(&failing, sources.allocator);
        DeliveryNativeOrigin sentinel = {0}; output = &sentinel;
        failure_at = allocation_count + ordinal;
        selected = delivery_native_origins(&failing, &frontend, &output, &error);
        assert(!selected && output == &sentinel && error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        failure_at = 0; source_arena_destroy(&failing);
    }
    finish();
}
/** Snapshots distinguish absent/empty/changed bytes and retain unavailable
 * guidance without running or parsing native build instructions.
 */
static void it_captures_exact_instruction_bytes_and_optional_guidance(void) {
    prepare("@!\none\n/test/fixture/v1.0.0/src/a\ntwo\n/test/fixture/v1.0.0/src/z\n");
    const char *path = "generated/project/packages/test/fixture/v1.0.0/src/a.cmake";
    const char *readme = "generated/project/packages/test/fixture/v1.0.0/README.md";
    file(path, "message(FATAL_ERROR \"Must not execute\")\n");
    directory(readme);
    const DeliveryNativeOrigin *origins = NULL; Diagnostic error = {0};
    bool selected = delivery_native_origins(&retained, &frontend, &origins, &error);
    assert(selected);
    SourceManager snapshots; source_manager_init(&snapshots, sources.allocator);
    const DeliveryNativeCapture *rows = NULL;
    bool captured = delivery_native_capture(&retained, &snapshots, origins, &rows, &error);
    assert(captured && rows != NULL && rows->next != NULL);
    assert(rows->cmake.state == DELIVERY_FILE_PRESENT && rows->next->cmake.state == DELIVERY_FILE_ABSENT);
    assert(rows->guidance == rows->next->guidance && rows->guidance->state == DELIVERY_FILE_UNAVAILABLE);
    assert(rows->guidance->issue == PATH_UNSUPPORTED && !strcmp(rows->guidance->path, readme));
    size_t length = source_length(rows->cmake.content);
    file(path, "");
    assert(source_length(rows->cmake.content) == length && length != 0);
    SourceArena other; source_arena_init(&other, sources.allocator);
    SourceManager newer; source_manager_init(&newer, sources.allocator);
    const DeliveryNativeCapture *changed = NULL;
    captured = delivery_native_capture(&other, &newer, origins, &changed, &error);
    assert(captured && changed->cmake.state == DELIVERY_FILE_PRESENT && !source_length(changed->cmake.content));
    assert(changed->cmake.content != rows->cmake.content);
    source_manager_destroy(&newer); source_arena_destroy(&other);
    int removed = remove(path); assert(!removed);
    removed = rmdir(readme); assert(!removed);
    source_manager_destroy(&snapshots); finish();
}
/** Required supplied instructions refuse shape errors, while every allocation
 * refusal preserves output and releases all private source/path owners.
 */
static void it_refuses_bad_instructions_and_every_capture_allocation_failure(void) {
    prepare("@!\none\n/test/fixture/v1.0.0/src/a\n");
    const char *path = "generated/project/packages/test/fixture/v1.0.0/src/a.cmake";
    directory(path);
    const DeliveryNativeOrigin *origins = NULL; Diagnostic error = {0};
    bool selected = delivery_native_origins(&retained, &frontend, &origins, &error); assert(selected);
    SourceManager snapshots; source_manager_init(&snapshots, sources.allocator);
    DeliveryNativeCapture sentinel = {0}; const DeliveryNativeCapture *output = &sentinel;
    bool captured = delivery_native_capture(&retained, &snapshots, origins, &output, &error);
    assert(!captured && output == &sentinel && error.code == DIAGNOSTIC_IO_FAILURE);
    assert(!strcmp(error.primary.path, path));
    int removed = rmdir(path); assert(!removed); file(path, "# authored bytes\n");
    directory("generated/project/packages/test/fixture/v1.0.0/src/a+");
    file("generated/project/packages/test/fixture/v1.0.0/src/a+/nested.cmake", "nested resource");
    file("generated/project/packages/test/fixture/v1.0.0/src/a.extra", "unknown companion");
    size_t before = allocation_count;
    captured = delivery_native_capture(&retained, &snapshots, origins, &output, &error); assert(captured);
    size_t total = allocation_count - before;
    for (size_t ordinal = 1; ordinal <= total; ordinal++) {
        SourceArena failing; source_arena_init(&failing, sources.allocator);
        SourceManager texts; source_manager_init(&texts, sources.allocator);
        output = &sentinel; failure_at = allocation_count + ordinal;
        captured = delivery_native_capture(&failing, &texts, origins, &output, &error);
        assert(!captured && output == &sentinel && error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        failure_at = 0; source_manager_destroy(&texts); source_arena_destroy(&failing);
    }
    removed = remove(path); assert(!removed);
    removed = remove("generated/project/packages/test/fixture/v1.0.0/src/a+/nested.cmake"); assert(!removed);
    removed = remove("generated/project/packages/test/fixture/v1.0.0/src/a.extra"); assert(!removed);
    removed = rmdir("generated/project/packages/test/fixture/v1.0.0/src/a+"); assert(!removed);
    source_manager_destroy(&snapshots); finish();
}
/** Raw capture retains arbitrary bytes and rejects traversal without publication. */
static void it_preserves_binary_files_and_rejects_invalid_child_requests(void) {
    prepare("@!\n");
    const char *parent = "generated/project/packages/test/fixture/v1.0.0/src";
    const char *path = "generated/project/packages/test/fixture/v1.0.0/src/raw.cmake";
    const unsigned char bytes[] = {0, 255, '\r', '\n'};
    FILE *stream = fopen(path, "wb"); assert(stream != NULL);
    size_t written = fwrite(bytes, 1, sizeof(bytes), stream); int closed = fclose(stream);
    assert(written == sizeof(bytes) && !closed);
    SourceManager snapshots; source_manager_init(&snapshots, sources.allocator);
    DeliveryFileSnapshot output = {0};
    SourceStatus status = delivery_file_capture(&retained, &snapshots, parent, "raw.cmake", &output);
    assert(status == SOURCE_OK && output.state == DELIVERY_FILE_PRESENT);
    assert(source_length(output.content) == sizeof(bytes) && !memcmp(source_bytes(output.content), bytes, sizeof(bytes)));
    const char *invalid[] = {"", ".", "..", "../raw.cmake", "a/b"};
    const SourceText *kept = output.content;
    for (size_t index = 0; index < sizeof(invalid) / sizeof(*invalid); index++) {
        status = delivery_file_capture(&retained, &snapshots, parent, invalid[index], &output);
        assert(status == SOURCE_INVALID_ARGUMENT && output.content == kept);
    }
    int removed = remove(path); assert(!removed);
    source_manager_destroy(&snapshots); finish();
}
/** Full declared units retain unknown companions, hidden files and nested raw
 * resources in deterministic order, without adding neighboring ownership.
 */
static void it_captures_the_complete_declared_resource_unit_and_refuses_symlinks(void) {
    prepare("@!\none\n/test/fixture/v1.0.0/src/a\n");
    const char *root = "generated/project/packages/test/fixture/v1.0.0/src/a+";
    const char *nested = "generated/project/packages/test/fixture/v1.0.0/src/a+/nested";
    const char *asset = "generated/project/packages/test/fixture/v1.0.0/src/a+/nested/.data";
    const char *companion = "generated/project/packages/test/fixture/v1.0.0/src/a.extra";
    const char *bad = "generated/project/packages/test/fixture/v1.0.0/src/a+/link";
    directory(root); directory(nested); file(asset, "payload"); file(companion, "companion");
    const DeliveryNativeOrigin *origins = NULL; Diagnostic error = {0};
    bool selected = delivery_native_origins(&retained, &frontend, &origins, &error); assert(selected);
    SourceManager snapshots; source_manager_init(&snapshots, sources.allocator);
    const DeliveryNativeCapture *rows = NULL;
    bool captured = delivery_native_capture(&retained, &snapshots, origins, &rows, &error); assert(captured);
    size_t count = 0, directories = 0; const char *previous = ""; bool found_asset = false, found_companion = false;
    for (const DeliveryNativeResource *item = rows->resources; item != NULL; item = item->next) {
        assert(strcmp(previous, item->file.path) < 0 && item->file.state == DELIVERY_FILE_PRESENT);
        previous = item->file.path; count++; directories += item->directory;
        if (!strcmp(item->file.path, asset)) {
            found_asset = true; assert(!item->directory && source_length(item->file.content) == 7);
            assert(!memcmp(source_bytes(item->file.content), "payload", 7));
        }
        if (!strcmp(item->file.path, companion)) { found_companion = true; }
    }
    assert(count == 4 && directories == 2 && found_asset && found_companion);
    int result = symlink("nested/.data", bad); assert(!result);
    DeliveryNativeCapture sentinel = {0}; const DeliveryNativeCapture *output = &sentinel;
    captured = delivery_native_capture(&retained, &snapshots, origins, &output, &error);
    assert(!captured && output == &sentinel && error.code == DIAGNOSTIC_IO_FAILURE && !strcmp(error.primary.path, bad));
    result = remove(bad); assert(!result); result = remove(asset); assert(!result);
    result = remove(companion); assert(!result); result = rmdir(nested); assert(!result);
    result = rmdir(root); assert(!result);
    source_manager_destroy(&snapshots); finish();
}
/** Unreadable optional guidance remains exact issue data; the same error on an
 * authored instruction refuses. These POSIX permission checks require nonroot.
 */
static void it_distinguishes_unreadable_guidance_from_unreadable_instructions(void) {
    prepare("@!\none\n/test/fixture/v1.0.0/src/a\n");
    const char *readme = "generated/project/packages/test/fixture/v1.0.0/README.md";
    const char *cmake = "generated/project/packages/test/fixture/v1.0.0/src/a.cmake";
    file(readme, "optional guidance"); file(cmake, "# required instruction");
    const DeliveryNativeOrigin *origins = NULL; Diagnostic error = {0};
    bool selected = delivery_native_origins(&retained, &frontend, &origins, &error); assert(selected);
    SourceManager snapshots; source_manager_init(&snapshots, sources.allocator);
    if (geteuid() != 0) {
        int result = chmod(readme, 0); assert(!result);
        const DeliveryNativeCapture *rows = NULL;
        bool captured = delivery_native_capture(&retained, &snapshots, origins, &rows, &error);
        assert(captured && rows->guidance->state == DELIVERY_FILE_UNAVAILABLE && rows->guidance->issue == PATH_IO_ERROR);
        result = chmod(cmake, 0); assert(!result);
        DeliveryNativeCapture sentinel = {0}; rows = &sentinel;
        captured = delivery_native_capture(&retained, &snapshots, origins, &rows, &error);
        assert(!captured && rows == &sentinel && error.code == DIAGNOSTIC_IO_FAILURE);
        result = chmod(readme, 0600); assert(!result); result = chmod(cmake, 0600); assert(!result);
    }
    int removed = remove(readme); assert(!removed); removed = remove(cmake); assert(!removed);
    source_manager_destroy(&snapshots); finish();
}
/** Native metadata reads sealed captured bytes after physical mutation, never
 * silently reopens the original file or publishes an unsealed view.
 */
static void it_exposes_only_sealed_authoritative_native_input_bytes(void) {
    prepare("@!\none\n/test/fixture/v1.0.0/src/a\n");
    const char *path = "generated/project/packages/test/fixture/v1.0.0/src/a.c";
    const SourceText *captured = project_inputs_content(&frontend.inputs, PROJECT_INPUT_FILE, path);
    assert(captured != NULL && source_length(captured) != 0);
    size_t length = source_length(captured);
    file(path, "mutated after snapshot");
    const SourceText *again = project_inputs_content(&frontend.inputs, PROJECT_INPUT_FILE, path);
    assert(again == captured && source_length(again) == length && source_bytes(again)[0] == '#');
    assert(project_inputs_content(&frontend.inputs, PROJECT_INPUT_FILE, "missing") == NULL);
    assert(project_inputs_content(NULL, PROJECT_INPUT_FILE, path) == NULL);
    frontend.inputs.sealed = false;
    assert(project_inputs_content(&frontend.inputs, PROJECT_INPUT_FILE, path) == NULL);
    frontend.inputs.sealed = true; finish();
}
/** Whole-version precedence keeps Unicode/multiword native instruction holes
 * absent even when the ignored external copy supplies both CMake and README.
 */
static void it_keeps_selected_version_holes_with_unicode_and_multiword_origins(void) {
    const char *name = "\xd1\x82\xd0\xb8\xd0\xbf name";
    prepare_sources("@!\nvalue\n/test/fixture/v1.0.0/src/\xd1\x82\xd0\xb8\xd0\xbf name\n");
    const char *parents[] = {"generated/project/dependencies", "generated/project/dependencies/test",
        "generated/project/dependencies/test/fixture", "generated/project/dependencies/test/fixture/v1.0.0",
        "generated/project/dependencies/test/fixture/v1.0.0/src"};
    for (size_t index = 0; index < sizeof(parents) / sizeof(*parents); index++) { directory(parents[index]); }
    file("generated/project/dependencies/test/fixture/v1.0.0/README.md", "Ignored guidance");
    char base[200], path[220];
    snprintf(base, sizeof(base), "generated/project/packages/test/fixture/v1.0.0/src/%s", name);
    const char *suffix[] = {"", ".c", ".h", ".abi"};
    const char *payload[] = {"@C\n", "#error Metadata only\n", "#error Metadata only\n", "+ initialize\n- destroy\n"};
    for (size_t index = 0; index < 4; index++) {
        snprintf(path, sizeof(path), "%s%s", base, suffix[index]); file(path, payload[index]);
    }
    snprintf(path, sizeof(path), "generated/project/dependencies/test/fixture/v1.0.0/src/%s.cmake", name);
    file(path, "message(FATAL_ERROR \"Ignored companion\")\n");
    directory("generated/project/packages/test/fixture/v1.0.0/src/a.cmake");
    begin_frontend();
    const DeliveryNativeOrigin *origins = NULL; Diagnostic error = {0};
    bool selected = delivery_native_origins(&retained, &frontend, &origins, &error); assert(selected);
    assert(origins != NULL && origins->next == NULL && !strcmp(source_path(origins->source->text), base));
    SourceManager snapshots; source_manager_init(&snapshots, sources.allocator);
    const DeliveryNativeCapture *rows = NULL;
    bool captured = delivery_native_capture(&retained, &snapshots, origins, &rows, &error);
    assert(captured && rows->cmake.state == DELIVERY_FILE_ABSENT && rows->guidance->state == DELIVERY_FILE_ABSENT);
    assert(rows->resources != NULL && rows->resources->file.state == DELIVERY_FILE_ABSENT);
    int removed = rmdir("generated/project/packages/test/fixture/v1.0.0/src/a.cmake"); assert(!removed);
    source_manager_destroy(&snapshots); finish();
}
/** A native Type reached only through a closed static constructor still belongs
 * to the actual emitted closure; unused native source definitions do not.
 */
static void it_retains_native_dependencies_of_selected_static_specializations(void) {
    prepare_sources("@!\ninstance\n/test/fixture/v1.0.0/src/factory\n .made\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/box", "@<\nitem\n.slot\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory",
        "@...\n.made\n/test/fixture/v1.0.0/src/box\n .slot < /test/fixture/v1.0.0/src/a\n");
    begin_frontend();
    const DeliveryNativeOrigin *origins = NULL; Diagnostic error = {0};
    bool selected = delivery_native_origins(&retained, &frontend, &origins, &error);
    assert(selected && origins != NULL && origins->next == NULL);
    assert(!strcmp(origins->source->logical_path, "/test/fixture/v1.0.0/src/a"));
    finish();
}
/** Ordinary-only components still report guidance; repeated planned types share
 * one version row and native observations are reused without rereading README.
 */
static void it_retains_ordinary_package_guidance_and_reuses_native_snapshots(void) {
    prepare("@!\n");
    SourceManager snapshots; source_manager_init(&snapshots, sources.allocator);
    Diagnostic error = {0}; const DeliveryVersionGuidance *versions = NULL;
    bool captured = delivery_version_guidance(&retained, &snapshots, &frontend, NULL, &versions, &error);
    assert(captured && versions != NULL && versions->next == NULL && versions->readme->state == DELIVERY_FILE_ABSENT);
    DeliveryVersionGuidance sentinel = {0}; const DeliveryVersionGuidance *output = &sentinel;
    size_t before = allocation_count;
    captured = delivery_version_guidance(&retained, &snapshots, &frontend, NULL, &versions, &error); assert(captured);
    size_t total = allocation_count - before;
    for (size_t ordinal = 1; ordinal <= total; ordinal++) {
        SourceArena failing; source_arena_init(&failing, sources.allocator);
        SourceManager texts; source_manager_init(&texts, sources.allocator);
        output = &sentinel; failure_at = allocation_count + ordinal;
        captured = delivery_version_guidance(&failing, &texts, &frontend, NULL, &output, &error);
        assert(!captured && output == &sentinel && error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        failure_at = 0; source_manager_destroy(&texts); source_arena_destroy(&failing);
    }
    source_manager_destroy(&snapshots); finish();
    prepare("@!\none\n/test/fixture/v1.0.0/src/a\ntwo\n/test/fixture/v1.0.0/src/z\n");
    source_manager_init(&snapshots, sources.allocator);
    const DeliveryNativeOrigin *origins = NULL; const DeliveryNativeCapture *native = NULL;
    bool selected = delivery_native_origins(&retained, &frontend, &origins, &error); assert(selected);
    captured = delivery_native_capture(&retained, &snapshots, origins, &native, &error); assert(captured);
    const char *readme = "generated/project/packages/test/fixture/v1.0.0/README.md";
    file(readme, "changed after authoritative native capture");
    captured = delivery_version_guidance(&retained, &snapshots, &frontend, native, &versions, &error);
    assert(captured && versions->next == NULL && versions->readme == native->guidance);
    assert(versions->readme->state == DELIVERY_FILE_ABSENT);
    assert(project_inputs_find(&frontend.inputs, PROJECT_INPUT_FILE, readme) == NULL);
    int removed = remove(readme); assert(!removed);
    source_manager_destroy(&snapshots); finish();
}
/** Explicitly run actual closure, owner isolation and allocation refusal cases. */
void run(void) {
    it_retains_ordinary_package_guidance_and_reuses_native_snapshots();
    it_keeps_selected_version_holes_with_unicode_and_multiword_origins();
    it_retains_native_dependencies_of_selected_static_specializations();
    it_exposes_only_sealed_authoritative_native_input_bytes();
    it_captures_the_complete_declared_resource_unit_and_refuses_symlinks();
    it_distinguishes_unreadable_guidance_from_unreadable_instructions();
    it_captures_exact_instruction_bytes_and_optional_guidance();
    it_refuses_bad_instructions_and_every_capture_allocation_failure();
    it_preserves_binary_files_and_rejects_invalid_child_requests();
    it_projects_only_actual_native_origins_in_identity_order();
    it_distinguishes_empty_projection_from_invalid_frontend();
    it_preserves_output_at_every_native_origin_allocation_failure();
}
/** All borrowed/owned fixture storage must be released before success. */
int main(void) { run(); return 0; }
