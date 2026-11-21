/** @file
 * @brief Verify original-source alpha renaming across distinct concrete template captures.
 * @see [T192 contract](../../../../../docs/implementation/static-editor-bindings.md)
 */
#include "../common.c"
/** Two concrete owners share original field/formal definitions, with independent substitutions. */
static void fixture(void) {
    project_fixture("@!\nfirst\n/test/fixture/v1.0.0/src/factory\n .left\nsecond\n/test/fixture/v1.0.0/src/factory\n .right\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/left","@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/right","@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/box","@<\nvalue\n.slot\n@>\nresult\n.slot\n value\n@!\nresult\n value\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory","@...\n.left\n/test/fixture/v1.0.0/src/box\n .slot < /test/fixture/v1.0.0/src/left\n.right\n/test/fixture/v1.0.0/src/box\n .slot < /test/fixture/v1.0.0/src/right\n");
}
/** One field edit covers both runtime owners, while preserving both distinct full Type keys. */
static void it_renames_one_original_field_across_distinct_specializations(void) {
    fixture();
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/box","value","new value"));
    assert(text_is(rename_plan.target.definition,"value"));
    assert(changed_file("/test/fixture/v1.0.0/src/box") && !changed_file("/test/fixture/v1.0.0/src/factory"));
    assert(validate());
    destroy_rename();
}
/** A static formal changes its own whole name and actual labels, without conflating captures. */
static void it_renames_static_formals_and_their_actual_constructor_labels(void) {
    fixture();
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/box",".slot",".тип.элемента-данных"));
    assert(changed_file("/test/fixture/v1.0.0/src/box") && changed_file("/test/fixture/v1.0.0/src/factory"));
    assert(validate());
    destroy_rename();
}
/** Graph outputs are source aliases, not runtime variables or new free parameters. */
static void it_renames_named_graph_outputs_and_ordinary_selection_sites(void) {
    fixture();
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/factory",".left",".selected result"));
    assert(changed_file("/test/fixture/v1.0.0/src/factory") && changed_file("/test/fixture/v1.0.0/src/main"));
    assert(validate());
    destroy_rename();
}
/** Complete static names use the production recognizer and never gain an implicit prefix. */
static void it_refuses_incomplete_or_malformed_static_replacement_names(void) {
    fixture();
    setup_rename();
    const char *invalid[] = {"slot",".","..slot"," .slot",".slot ",".two  words",".a..b",".bad--name",".a/b"};
    for (size_t index = 0; index < sizeof(invalid)/sizeof(*invalid); index++) {
        assert(!prepare("/test/fixture/v1.0.0/src/box",".slot",invalid[index]));
        assert(!rename_plan.prepared && !rename_plan.first && diagnostic_valid(&rename_plan.diagnostic));
        editor_rename_destroy(&rename_plan);
        editor_rename_init(&rename_plan,sources.allocator);
    }
    destroy_rename();
}
/** An open template remains a source declaration even without any concrete runtime owner. */
static void it_renames_uninstantiated_template_fields_without_fabricating_runtime_types(void) {
    fixture();
    file("generated/project/packages/test/fixture/v1.0.0/src/main","@!\n");
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/box","value","uninstantiated value"));
    assert(!rename_plan.target.symbol && rename_plan.target.original);
    assert(validate());
    destroy_rename();
}
/** A shared higher-order label cannot be edited for only one of its actual constructor origins. */
static void it_refuses_multi_origin_labels_and_single_origin_edits_that_would_change_them(void) {
    fixture();
    const char *box = "@<\nvalue\n.slot\n@>\nresult\n.slot\n value\n@!\nresult\n value\n";
    file("generated/project/packages/test/fixture/v1.0.0/src/other box",box);
    file("generated/project/packages/test/fixture/v1.0.0/src/apply","@...\n.out\n.family\n .slot < .type\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory","@...\n/test/fixture/v1.0.0/src/apply\n .family < /test/fixture/v1.0.0/src/box\n .type < /test/fixture/v1.0.0/src/left\n .out > .left\n/test/fixture/v1.0.0/src/apply\n .family < /test/fixture/v1.0.0/src/other box\n .type < /test/fixture/v1.0.0/src/right\n .out > .right\n");
    setup_rename();
    assert(!prepare("/test/fixture/v1.0.0/src/apply",".slot <",".new slot") && !rename_plan.prepared);
    editor_rename_destroy(&rename_plan);
    editor_rename_init(&rename_plan,sources.allocator);
    assert(!prepare("/test/fixture/v1.0.0/src/box",".slot",".new slot") && !rename_plan.prepared);
    assert(rename_plan.diagnostic.code == DIAGNOSTIC_RENAME_CONFLICT);
    destroy_rename();
}

/** Forward graph uses and repeated receipts retain one original alias declaration after renaming. */
static void it_renames_forward_graph_bindings_and_defining_and_equality_receipts(void) {
    fixture();
    file("generated/project/packages/test/fixture/v1.0.0/src/main","@!\ndata\n/test/fixture/v1.0.0/src/factory\n .received\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/producer","@...\n.out\n/test/fixture/v1.0.0/src/box\n .slot < /test/fixture/v1.0.0/src/left\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory","@...\n.forward\n.later\n.later\n/test/fixture/v1.0.0/src/box\n .slot < /test/fixture/v1.0.0/src/left\n/test/fixture/v1.0.0/src/producer\n .out > .received\n/test/fixture/v1.0.0/src/producer\n .out > .received\n");
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/factory",".later",".new later") && validate());
    const EditorRenameFile *changed = changed_file("/test/fixture/v1.0.0/src/factory");
    assert(changed && changed->edit_count == 2);
    editor_rename_destroy(&rename_plan);
    editor_rename_init(&rename_plan,sources.allocator);
    const ModuleSource *source = NULL;
    assert(module_catalog_find(&analysis.frontend.project.catalog,"/test/fixture/v1.0.0/src/factory",&source) == MODULE_OK);
    size_t offset = point(source->text,"> .received")+2;
    assert(editor_rename_prepare(&rename_plan,&analysis,"/test/fixture/v1.0.0/src/factory",offset,
        (const unsigned char *)".new received",strlen(".new received")));
    assert(validate());
    assert(changed_file("/test/fixture/v1.0.0/src/factory")->edit_count == 2 && changed_file("/test/fixture/v1.0.0/src/main")->edit_count == 1);
    destroy_rename();
}

#include "support/refusals.c"

/** Checked multi-file renaming follows every nested declaration selector/input
 * rather than treating its indentation as a different scope or parameter.
 */
static void it_renames_nested_declaration_labels_and_selectors_with_checked_validation(void) {
    for (size_t output = 0; output < 2; output++) {
        project_fixture("@!\nvalue&\n./factory\n .input < ./factory\n  .input < ./cell\n  .out\n .out\n");
        file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.out\n.input\n");
        setup_rename();
        assert(prepare("/test/fixture/v1.0.0/src/factory", output ? ".out" : ".input",
            output ? ".selected type" : ".input type"));
        assert(changed_file("/test/fixture/v1.0.0/src/main") && changed_file("/test/fixture/v1.0.0/src/factory"));
        assert(validate());
        destroy_rename();
    }
}

/** Run actual compiler/candidate graph validation rather than testing helper equality in isolation. */
static void run(void) {
    it_renames_one_original_field_across_distinct_specializations();
    it_renames_static_formals_and_their_actual_constructor_labels();
    it_renames_named_graph_outputs_and_ordinary_selection_sites();
    it_renames_nested_declaration_labels_and_selectors_with_checked_validation();
    it_refuses_incomplete_or_malformed_static_replacement_names();
    it_renames_uninstantiated_template_fields_without_fabricating_runtime_types();
    it_refuses_multi_origin_labels_and_single_origin_edits_that_would_change_them();
    it_renames_forward_graph_bindings_and_defining_and_equality_receipts();
    it_unwinds_every_static_candidate_correspondence_allocation();
}
/** All borrowed source graphs retire before counted ownership checks. */
int main(void) { run(); return 0; }
