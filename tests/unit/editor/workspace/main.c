/** @file
 * @brief Verify complete active workspace analysis, unused importers, opaque drafts and copied configuration ownership.
 */
#include "../common.c"
/** Analyze the exact configured inventory rather than just one entry's dependency closure. */
static bool workspace(void) {
    ModuleRoot root = {"generated/project",NULL,0,MODULE_ROOT_PROJECT};
    return editor_workspace_build(&analysis,&root,1,".main");
}
/** Independently active importers contribute checked references even when the selected entry never loads them. */
static void it_checks_unused_active_importers_without_parsing_incomplete_drafts(void) {
    project_fixture("@!\n");
    file("generated/project/external","@!\nworker\n.worker\nworker !\n argument < value\nvalue\n.cell\n");
    file("generated/project/draft","unfinished definition\n .missing\n");
    setup_editor();
    assert(workspace() && analysis.indexed && analysis.workspace);
    const SemanticType *external = semantic_find_type(&analysis.frontend.semantic,".external");
    assert(external != NULL && semantic_find_type(&analysis.frontend.semantic,".draft") == NULL);
    EditorDefinition definition;
    size_t label = point(external->module->source->text,"argument <");
    assert(editor_definition(&analysis,".external",label,&definition));
    assert(text_is(definition.symbol->name,"argument"));
    assert(strcmp(definition.symbol->owner->module->source->logical_path,".worker") == 0);
    destroy_editor();
}
/** Open active templates remain checked source definitions without fabricated runtime Types. */
static void it_preserves_uninstantiated_templates_without_default_workspace_instances(void) {
    project_fixture("@!\n");
    file("generated/project/generic", "@<\ninput\n...slot\n@!\n");
    setup_editor();
    assert(workspace() && analysis.indexed && analysis.workspace);
    assert(project_find(&analysis.frontend.project, ".generic"));
    assert(!semantic_find_type(&analysis.frontend.semantic, ".generic"));
    destroy_editor();
}

/** A broken unused active module is a real workspace blocker, while ordinary entry-only analysis remains independent. */
static void it_rejects_invalid_unused_active_modules_without_publishing_partial_identities(void) {
    project_fixture("@!\n");
    file("generated/project/broken","@!\nunknown !\n");
    setup_editor();
    assert(analyze(&analysis) && analysis.indexed && !analysis.workspace);
    destroy_editor();
    setup_editor();
    assert(!workspace() && !analysis.indexed && !analysis.workspace && analysis.state == EDITOR_INVALID);
    assert(diagnostic_valid(&analysis.diagnostic));
    destroy_editor();
    int removed = remove("generated/project/broken");
    assert(removed == 0);
}
/** Transient caller options may disappear or change after construction without changing workspace roots or entry. */
static void it_owns_root_configuration_and_releases_every_failed_workspace_prefix(void) {
    project_fixture("@!\nvalue\n.cell\n");
    setup_editor();
    char path[] = "generated/project", entry[] = ".main", exclusion[] = "unused";
    const char *exclusions[] = {exclusion};
    ModuleRoot root = {path,exclusions,1,MODULE_ROOT_PROJECT};
    assert(editor_workspace_build(&analysis,&root,1,entry));
    path[0] = 'X'; entry[1] = 'X'; exclusion[0] = 'X';
    assert(strcmp(analysis.roots[0].path,"generated/project") == 0 && strcmp(analysis.entry,".main") == 0);
    assert(strcmp(analysis.roots[0].excluded[0],"unused") == 0);
    assert(point(analysis.selected->text,"value") != 0);
    destroy_editor();
    setup_editor();
    assert(workspace());
    size_t total = allocation_count;
    destroy_editor();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_editor();
        failure_at = failure;
        bool valid = workspace();
        if (valid || analysis.diagnostic.code != DIAGNOSTIC_OUT_OF_MEMORY) {
            fprintf(stderr,"Workspace ordinal %zu/%zu: valid=%u code=%u\n",failure,total,(unsigned)valid,(unsigned)analysis.diagnostic.code);
        }
        assert(!valid && !analysis.indexed && !analysis.workspace && analysis.diagnostic.code == DIAGNOSTIC_OUT_OF_MEMORY);
        destroy_editor();
    }
}
/** Each case uses the same production catalog, activation classifier, parser, resolver and planner. */
void run(void) {
    it_checks_unused_active_importers_without_parsing_incomplete_drafts();
    it_preserves_uninstantiated_templates_without_default_workspace_instances();
    it_rejects_invalid_unused_active_modules_without_publishing_partial_identities();
    it_owns_root_configuration_and_releases_every_failed_workspace_prefix();
}
/** All copied roots, draft/active source records and failed semantic prefixes retire before success. */
int main(void) { run(); return EXIT_SUCCESS; }
