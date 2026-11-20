/** @file
 * @brief Verify original-source alpha renaming across distinct concrete template captures.
 * @see [T192 contract](../../../../../docs/implementation/static-editor-bindings.md)
 */
#include "../common.c"
/** Two concrete owners share original field/formal definitions, with independent substitutions. */
static void fixture(void) {
    project_fixture("@!\nfirst\n.factory\n ...left\nsecond\n.factory\n ...right\n");
    file("generated/project/left","@!\n");
    file("generated/project/right","@!\n");
    file("generated/project/box","@<\nvalue\n...slot\n@>\nresult\n...slot\n value\n@!\nresult\n value\n");
    file("generated/project/factory","@...\n...left\n.box\n ...slot < .left\n...right\n.box\n ...slot < .right\n");
}
/** One field edit covers both runtime owners, while preserving both distinct full Type keys. */
static void it_renames_one_original_field_across_distinct_specializations(void) {
    fixture();
    setup_rename();
    assert(prepare(".box","value","new value"));
    assert(text_is(rename_plan.target.definition,"value"));
    assert(changed_file(".box") && !changed_file(".factory"));
    assert(validate());
    destroy_rename();
}
/** A static formal changes its own whole name and actual labels, without conflating captures. */
static void it_renames_static_formals_and_their_actual_constructor_labels(void) {
    fixture();
    setup_rename();
    assert(prepare(".box","...slot","...тип.элемента-данных"));
    assert(changed_file(".box") && changed_file(".factory"));
    assert(validate());
    destroy_rename();
}
/** Graph outputs are source aliases, not runtime variables or new free parameters. */
static void it_renames_named_graph_outputs_and_ordinary_selection_sites(void) {
    fixture();
    setup_rename();
    assert(prepare(".factory","...left","...selected result"));
    assert(changed_file(".factory") && changed_file(".main"));
    assert(validate());
    destroy_rename();
}
/** Complete static names use the production recognizer and never gain an implicit prefix. */
static void it_refuses_incomplete_or_malformed_static_replacement_names(void) {
    fixture();
    setup_rename();
    const char *invalid[] = {"slot","...","....slot"," ...slot","...slot ","...two  words","...a..b","...bad--name"};
    for (size_t index = 0; index < sizeof(invalid)/sizeof(*invalid); index++) {
        assert(!prepare(".box","...slot",invalid[index]));
        assert(!rename_plan.prepared && !rename_plan.first && diagnostic_valid(&rename_plan.diagnostic));
        editor_rename_destroy(&rename_plan);
        editor_rename_init(&rename_plan,sources.allocator);
    }
    destroy_rename();
}
/** An open template remains a source declaration even without any concrete runtime owner. */
static void it_renames_uninstantiated_template_fields_without_fabricating_runtime_types(void) {
    fixture();
    file("generated/project/main","@!\n");
    setup_rename();
    assert(prepare(".box","value","uninstantiated value"));
    assert(!rename_plan.target.symbol && rename_plan.target.original);
    assert(validate());
    destroy_rename();
}
/** A shared higher-order label cannot be edited for only one of its actual constructor origins. */
static void it_refuses_multi_origin_labels_and_single_origin_edits_that_would_change_them(void) {
    fixture();
    const char *box = "@<\nvalue\n...slot\n@>\nresult\n...slot\n value\n@!\nresult\n value\n";
    file("generated/project/other box",box);
    file("generated/project/apply","@...\n...out\n...family\n ...slot < ...type\n");
    file("generated/project/factory","@...\n.apply\n ...family < .box\n ...type < .left\n ...out > ...left\n.apply\n ...family < .other box\n ...type < .right\n ...out > ...right\n");
    setup_rename();
    assert(!prepare(".apply","...slot <","...new slot") && !rename_plan.prepared);
    editor_rename_destroy(&rename_plan);
    editor_rename_init(&rename_plan,sources.allocator);
    assert(!prepare(".box","...slot","...new slot") && !rename_plan.prepared);
    assert(rename_plan.diagnostic.code == DIAGNOSTIC_RENAME_CONFLICT);
    destroy_rename();
}

/** Forward graph uses and repeated receipts retain one original alias declaration after renaming. */
static void it_renames_forward_graph_bindings_and_defining_and_equality_receipts(void) {
    fixture();
    file("generated/project/main","@!\ndata\n.factory\n ...received\n");
    file("generated/project/producer","@...\n...out\n.box\n ...slot < .left\n");
    file("generated/project/factory","@...\n...forward\n...later\n...later\n.box\n ...slot < .left\n.producer\n ...out > ...received\n.producer\n ...out > ...received\n");
    setup_rename();
    assert(prepare(".factory","...later","...new later") && validate());
    const EditorRenameFile *changed = changed_file(".factory");
    assert(changed && changed->edit_count == 2);
    editor_rename_destroy(&rename_plan);
    editor_rename_init(&rename_plan,sources.allocator);
    const ModuleSource *source = NULL;
    assert(module_catalog_find(&analysis.frontend.project.catalog,".factory",&source) == MODULE_OK);
    size_t offset = point(source->text,"> ...received")+2;
    assert(editor_rename_prepare(&rename_plan,&analysis,".factory",offset,
        (const unsigned char *)"...new received",strlen("...new received")));
    assert(validate());
    assert(changed_file(".factory")->edit_count == 2 && changed_file(".main")->edit_count == 1);
    destroy_rename();
}

#include "support/refusals.c"

/** Run actual compiler/candidate graph validation rather than testing helper equality in isolation. */
static void run(void) {
    it_renames_one_original_field_across_distinct_specializations();
    it_renames_static_formals_and_their_actual_constructor_labels();
    it_renames_named_graph_outputs_and_ordinary_selection_sites();
    it_refuses_incomplete_or_malformed_static_replacement_names();
    it_renames_uninstantiated_template_fields_without_fabricating_runtime_types();
    it_refuses_multi_origin_labels_and_single_origin_edits_that_would_change_them();
    it_renames_forward_graph_bindings_and_defining_and_equality_receipts();
    it_unwinds_every_static_candidate_correspondence_allocation();
}
/** All borrowed source graphs retire before counted ownership checks. */
int main(void) { run(); return 0; }
