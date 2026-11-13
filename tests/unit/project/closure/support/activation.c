/** @file
 * @brief Compose parsed activation, required native companions and exact input invalidation.
 */
#include "project/inputs/inputs.h"
/** Prepare one required native module beside ordinary/unused sources with deliberately unusable companions. */
static void prepare_activation(void) {
    directory("generated/activation");
    file("generated/activation/main", "@!\nordinary\n.ordinary\nnative\n.native\n");
    file("generated/activation/ordinary", "@!\n");
    file("generated/activation/ordinary.abi", "\xff");
    file("generated/activation/native", "@C\n");
    file("generated/activation/native.c", "/* native body */\n");
    file("generated/activation/native.h", "/* native header */\n");
    file("generated/activation/native.abi", "+ initialize\n- destroy\n");
    file("generated/activation/unused", "@C\n");
    file("generated/activation/unused.abi", "\xff");
    file("generated/activation/draft", "unfinished[\n");
    file("generated/activation/draft.abi", "\xff");
}
/** Source catalog registration alone never selects companions for an ordinary, unused native or draft source. */
static void it_selects_native_companions_only_for_required_parsed_native_modules(void) {
    prepare_activation();
    setup_project();
    ProjectStatus status = load_root("generated/activation");
    assert(status == PROJECT_OK && module_count() == 3);
    const ProjectModule *native = project_find(&project, ".native");
    assert(native->source->native_loaded && native->source->native_c != NULL
        && native->source->native_header != NULL && native->source->native_abi != NULL);
    const char *ignored[] = {".ordinary", ".unused", ".draft"};
    for (size_t index = 0; index < 3; index++) {
        const ModuleSource *source = NULL;
        ModuleStatus found = module_catalog_find(&project.catalog, ignored[index], &source);
        assert(found == MODULE_OK && !source->native_loaded && source->native_abi == NULL);
        assert(source->native_c == NULL && source->native_header == NULL);
    }
    destroy_project();
    file("generated/activation/native.abi", "+ init\n\xff");
    setup_project();
    status = load_root("generated/activation");
    assert(status == PROJECT_CATALOG_ERROR && project.error.code == DIAGNOSTIC_INVALID_UTF8);
    assert(diagnostic_valid(&project.error) && strcmp(source_path(project.error.primary.span.source), "generated/activation/native.abi") == 0);
    assert(project.error.related_count == 1 && layout_text_is(project.error.related[0].location.span, ".native"));
    destroy_project();
}
/** Required drafts become available only after activation; stale native companions remain irrelevant for an Ama implementation. */
static void it_reloads_required_drafts_after_activation_without_reusing_an_inactive_record(void) {
    prepare_activation();
    file("generated/activation/main", "@!\nvalue\n.draft\n");
    setup_project();
    ProjectStatus status = load_root("generated/activation");
    assert(status == PROJECT_DRAFT && layout_text_is(project.error.primary.span, ".draft"));
    destroy_project();
    file("generated/activation/draft", "@!\n");
    setup_project();
    status = load_root("generated/activation");
    assert(status == PROJECT_OK && module_count() == 2);
    assert(project_find(&project, ".draft")->syntax.sections.kind == MODULE_AMA);
    assert(!project_find(&project, ".draft")->source->native_loaded);
    destroy_project();
    file("generated/activation/draft", "@C\n");
    setup_project();
    status = load_root("generated/activation");
    assert(status == PROJECT_CATALOG_ERROR && project.error.code == DIAGNOSTIC_INVALID_UTF8);
    destroy_project();
    file("generated/activation/draft.abi", "+ initialize\n- destroy\n");
    setup_project();
    status = load_root("generated/activation");
    assert(status == PROJECT_OK && project_find(&project, ".draft")->source->native_loaded);
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
    assert(project_inputs_find(&before, PROJECT_INPUT_FILE, "generated/activation/ordinary.abi") == NULL);
    assert(project_inputs_find(&before, PROJECT_INPUT_FILE, "generated/activation/native.c") != NULL);
    file("generated/activation/native", "@!\n");
    file("generated/activation/native.abi", "\xff");
    Project after_project;
    project_init(&after_project, sources.allocator);
    loaded = project_load(&after_project, &root, 1, ".main");
    assert(loaded == PROJECT_OK);
    ProjectInputs after;
    project_inputs_init(&after, sources.allocator);
    status = project_inputs_capture(&after, &after_project, &root, 1);
    assert(status == SOURCE_OK);
    project_inputs_seal(&after);
    status = project_inputs_compare(&after, &before);
    assert(status == SOURCE_OK);
    bool changed = false;
    bool found = project_inputs_changed(&after, PROJECT_INPUT_MODULE, ".native", &changed);
    assert(found && changed);
    found = project_inputs_changed(&after, PROJECT_INPUT_MODULE, ".main", &changed);
    assert(found && changed);
    found = project_inputs_changed(&after, PROJECT_INPUT_MODULE, ".ordinary", &changed);
    assert(found && !changed);
    assert(project_inputs_find(&after, PROJECT_INPUT_FILE, "generated/activation/native.c") == NULL);
    assert(project_inputs_find(&after, PROJECT_INPUT_FILE, "generated/activation/native.abi") == NULL);
    project_inputs_destroy(&after);
    project_destroy(&after_project);
    project_inputs_destroy(&before);
    destroy_project();
}
