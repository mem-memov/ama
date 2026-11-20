/** @file
 * @brief Compose parsed activation, required native companions and exact input invalidation.
 */
#include "project/inputs/inputs.h"
/** Prepare one required native module beside ordinary/unused sources with deliberately unusable companions. */
static void prepare_activation(void) {
    fixture_package_prepare("generated/activation", "fixture");
    file("generated/activation/packages/test/fixture/v1.0.0/src/main", "@!\nordinary\n/test/fixture/v1.0.0/src/ordinary\nnative\n/test/fixture/v1.0.0/src/native\n");
    file("generated/activation/packages/test/fixture/v1.0.0/src/ordinary", "@!\n");
    file("generated/activation/packages/test/fixture/v1.0.0/src/ordinary.abi", "\xff");
    file("generated/activation/packages/test/fixture/v1.0.0/src/native", "@C\n");
    file("generated/activation/packages/test/fixture/v1.0.0/src/native.c", "/* native body */\n");
    file("generated/activation/packages/test/fixture/v1.0.0/src/native.h", "/* native header */\n");
    file("generated/activation/packages/test/fixture/v1.0.0/src/native.abi", "+ initialize\n- destroy\n");
    file("generated/activation/packages/test/fixture/v1.0.0/src/unused", "@C\n");
    file("generated/activation/packages/test/fixture/v1.0.0/src/unused.abi", "\xff");
    file("generated/activation/packages/test/fixture/v1.0.0/src/draft", "unfinished[\n");
    file("generated/activation/packages/test/fixture/v1.0.0/src/draft.abi", "\xff");
}
/** Source catalog registration alone never selects companions for an ordinary, unused native or draft source. */
static void it_selects_native_companions_only_for_required_parsed_native_modules(void) {
    prepare_activation();
    setup_project();
    ProjectStatus status = load_root("generated/activation");
    assert(status == PROJECT_OK && module_count() == 3);
    const ProjectModule *native = project_find(&project, "/test/fixture/v1.0.0/src/native");
    assert(native->source->native_loaded && native->source->native_c != NULL
        && native->source->native_header != NULL && native->source->native_abi != NULL);
    const char *ignored[] = {"/test/fixture/v1.0.0/src/ordinary", "/test/fixture/v1.0.0/src/unused", "/test/fixture/v1.0.0/src/draft"};
    for (size_t index = 0; index < 3; index++) {
        const ModuleSource *source = NULL;
        ModuleStatus found = module_catalog_find(&project.catalog, ignored[index], &source);
        assert(found == MODULE_OK && !source->native_loaded && source->native_abi == NULL);
        assert(source->native_c == NULL && source->native_header == NULL);
    }
    destroy_project();
    file("generated/activation/packages/test/fixture/v1.0.0/src/native.abi", "+ init\n\xff");
    setup_project();
    status = load_root("generated/activation");
    assert(status == PROJECT_CATALOG_ERROR && project.error.code == DIAGNOSTIC_INVALID_UTF8);
    assert(diagnostic_valid(&project.error) && strcmp(source_path(project.error.primary.span.source), "generated/activation/packages/test/fixture/v1.0.0/src/native.abi") == 0);
    assert(project.error.related_count == 1 && layout_text_is(project.error.related[0].location.span, "/test/fixture/v1.0.0/src/native"));
    destroy_project();
}
/** Required drafts become available only after activation; stale native companions remain irrelevant for an Ama implementation. */
static void it_reloads_required_drafts_after_activation_without_reusing_an_inactive_record(void) {
    prepare_activation();
    file("generated/activation/packages/test/fixture/v1.0.0/src/main", "@!\nvalue\n/test/fixture/v1.0.0/src/draft\n");
    setup_project();
    ProjectStatus status = load_root("generated/activation");
    assert(status == PROJECT_DRAFT && layout_text_is(project.error.primary.span, "/test/fixture/v1.0.0/src/draft"));
    destroy_project();
    file("generated/activation/packages/test/fixture/v1.0.0/src/draft", "@!\n");
    setup_project();
    status = load_root("generated/activation");
    assert(status == PROJECT_OK && module_count() == 2);
    assert(project_find(&project, "/test/fixture/v1.0.0/src/draft")->syntax.sections.kind == MODULE_AMA);
    assert(!project_find(&project, "/test/fixture/v1.0.0/src/draft")->source->native_loaded);
    destroy_project();
    file("generated/activation/packages/test/fixture/v1.0.0/src/draft", "@C\n");
    setup_project();
    status = load_root("generated/activation");
    assert(status == PROJECT_CATALOG_ERROR && project.error.code == DIAGNOSTIC_INVALID_UTF8);
    destroy_project();
    file("generated/activation/packages/test/fixture/v1.0.0/src/draft.abi", "+ initialize\n- destroy\n");
    setup_project();
    status = load_root("generated/activation");
    assert(status == PROJECT_OK && project_find(&project, "/test/fixture/v1.0.0/src/draft")->source->native_loaded);
    destroy_project();
}
/** Native-to-Ama changes remove contributing native inputs and invalidate consumers; ignored ordinary companions never become build dependencies. */
static void it_invalidates_activation_changes_and_tracks_only_contributing_companions(void) {
    prepare_activation();
    setup_project();
    ProjectStatus loaded = load_root("generated/activation");
    assert(loaded == PROJECT_OK);
    ModuleRoot root = {"generated/activation", NULL, 0, MODULE_ROOT_PROJECT};
    ProjectInputs before;
    project_inputs_init(&before, sources.allocator);
    SourceStatus status = project_inputs_capture(&before, &project, &root, 1);
    assert(status == SOURCE_OK);
    project_inputs_seal(&before);
    assert(project_inputs_find(&before, PROJECT_INPUT_FILE, "generated/activation/packages/test/fixture/v1.0.0/src/ordinary.abi") == NULL);
    assert(project_inputs_find(&before, PROJECT_INPUT_FILE, "generated/activation/packages/test/fixture/v1.0.0/src/native.c") != NULL);
    file("generated/activation/packages/test/fixture/v1.0.0/src/native", "@!\n");
    file("generated/activation/packages/test/fixture/v1.0.0/src/native.abi", "\xff");
    Project after_project;
    project_init(&after_project, sources.allocator);
    loaded = project_load(&after_project, &root, 1, "/test/fixture/v1.0.0/src/main");
    assert(loaded == PROJECT_OK);
    ProjectInputs after;
    project_inputs_init(&after, sources.allocator);
    status = project_inputs_capture(&after, &after_project, &root, 1);
    assert(status == SOURCE_OK);
    project_inputs_seal(&after);
    status = project_inputs_compare(&after, &before);
    assert(status == SOURCE_OK);
    bool changed = false;
    bool found = project_inputs_changed(&after, PROJECT_INPUT_MODULE, "/test/fixture/v1.0.0/src/native", &changed);
    assert(found && changed);
    found = project_inputs_changed(&after, PROJECT_INPUT_MODULE, "/test/fixture/v1.0.0/src/main", &changed);
    assert(found && changed);
    found = project_inputs_changed(&after, PROJECT_INPUT_MODULE, "/test/fixture/v1.0.0/src/ordinary", &changed);
    assert(found && !changed);
    assert(project_inputs_find(&after, PROJECT_INPUT_FILE, "generated/activation/packages/test/fixture/v1.0.0/src/native.c") == NULL);
    assert(project_inputs_find(&after, PROJECT_INPUT_FILE, "generated/activation/packages/test/fixture/v1.0.0/src/native.abi") == NULL);
    project_inputs_destroy(&after);
    project_destroy(&after_project);
    project_inputs_destroy(&before);
    destroy_project();
}
