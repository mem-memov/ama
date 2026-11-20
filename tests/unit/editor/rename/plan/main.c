/** @file
 * @brief Check complete identity-selected edits, unrelated byte preservation and ordinary valid candidate graphs.
 */
#include "../common.c"
/** Existing same-spelled caller values do not belong to a renamed callee parameter. */
static void it_renames_callee_labels_in_unused_importers_without_changing_caller_values(void) {
    project_fixture("@!\nworker\n.worker\nargument\n.cell\nworker !\n argument < argument\n");
    file("generated/project/external","@!\nworker\n.worker\nargument\n.cell\nworker !\n argument < argument\n");
    setup_rename();
    assert(prepare(".worker","argument","new argument"));
    assert(rename_plan.prepared && changed_file(".worker") != NULL && changed_file(".external") != NULL);
    const EditorRenameFile *main = changed_file(".main");
    assert(main != NULL && strcmp((const char *)source_bytes(main->replacement),
        "@!\nworker\n.worker\nargument\n.cell\nworker !\n new argument < argument\n") == 0);
    assert(validate() && rename_plan.validated);
    assert(text_is(rename_plan.target.definition,"argument"));
    destroy_rename();
}
/** Qualified constant labels and declaration names change while paths/comments and another owner remain exact. */
static void it_renames_qualified_constants_across_files_and_preserves_unrelated_bytes(void) {
    project_fixture("@!\r\nshared\r\n.cell\r\n.palette\r\n shared >> copied\r\n# shared .palette  \r\n");
    file("generated/project/external","@!\n.palette\n shared >> value\n");
    setup_rename();
    assert(prepare(".palette","shared","общее значение"));
    const EditorRenameFile *main = changed_file(".main");
    assert(main != NULL && strcmp((const char *)source_bytes(main->replacement),
        "@!\r\nshared\r\n.cell\r\n.palette\r\n общее значение >> copied\r\n# shared .palette  \r\n") == 0);
    assert(changed_file(".external") != NULL && changed_file(".palette") != NULL);
    assert(validate() && point(analysis.selected->text,"shared") != 0);
    destroy_rename();
}
/** Reference/list suffixes remain compact and a returned reference receiver keeps its own distinct identity. */
static void it_preserves_reference_list_suffixes_and_same_name_noops(void) {
    project_fixture("@!\nvalues&[]\n.cell\ncopy&[]\n values&[]\n# values&[]\n");
    setup_rename();
    assert(prepare(".main","values&[]","new values"));
    const EditorRenameFile *main = changed_file(".main");
    assert(main != NULL && strcmp((const char *)source_bytes(main->replacement),
        "@!\nnew values&[]\n.cell\ncopy&[]\n new values&[]\n# values&[]\n") == 0);
    assert(validate());
    editor_rename_destroy(&rename_plan);
    editor_rename_init(&rename_plan,sources.allocator);
    assert(prepare(".main","values&[]","values") && rename_plan.first == NULL);
    assert(validate());
    destroy_rename();
}
/** Literal payloads and comments keep old words even in a file containing actual renamed declarations and accesses. */
static void it_preserves_literal_and_comment_bytes_with_real_standard_text_metadata(void) {
    project_fixture("@!\r\nvalue\r\n.cell\r\nmessage\r\n..text\r\n |value  # value  \r\ncopy\r\n value\r\n# value  \r\n");
    ModuleRoot roots[] = {{"generated/project",NULL,0,MODULE_ROOT_PROJECT},{TEST_STANDARD_ROOT,NULL,0,MODULE_ROOT_STANDARD}};
    setup_rename_roots(roots,2);
    assert(prepare(".main","value","новое имя"));
    const EditorRenameFile *main = changed_file(".main");
    assert(main != NULL && strcmp((const char *)source_bytes(main->replacement),
        "@!\r\nновое имя\r\n.cell\r\nmessage\r\n..text\r\n |value  # value  \r\ncopy\r\n новое имя\r\n# value  \r\n") == 0);
    assert(validate());
    destroy_rename();
}
/** Conditional/loop links and selected-method refusal actions retain their exact caller-side variable identity. */
static void it_preserves_branch_loop_and_selected_method_refusal_graphs(void) {
    project_fixture("@!\nvalue\n.cell\ncondition\n..std.condition\n |1\nworker\n.worker\ncondition\n?\n positive\n  value\ncondition\n:\n negative\n  value\ncondition\n{\n loop copy\n  value\n ^\n}\nworker\n result& !\n :\n  fallback\n   value\n");
    ModuleRoot roots[] = {{"generated/project",NULL,0,MODULE_ROOT_PROJECT},{TEST_STANDARD_ROOT,NULL,0,MODULE_ROOT_STANDARD}};
    setup_rename_roots(roots,2);
    assert(prepare(".main","value","renamed value") && validate());
    assert(changed_file(".main") != NULL);
    destroy_rename();
}
/** Index operands rename as caller declarations; a same-spelled intrinsic label remains its fixed virtual interface field. */
static void it_preserves_intrinsic_labels_while_renaming_index_operands(void) {
    project_fixture("@!\nposition\n..std.index\n |0\nordinal\n..integer\n |0\nvalues[]\n.cell\nvalues[position] !\n:\nvalues[]\n insert !\n  position < ordinal\n");
    ModuleRoot roots[] = {{"generated/project",NULL,0,MODULE_ROOT_PROJECT},{TEST_STANDARD_ROOT,NULL,0,MODULE_ROOT_STANDARD}};
    setup_rename_roots(roots,2);
    assert(prepare(".main","position","cursor") && validate());
    const EditorRenameFile *main = changed_file(".main");
    assert(main != NULL && strstr((const char *)source_bytes(main->replacement),"values[cursor] !") != NULL);
    assert(strstr((const char *)source_bytes(main->replacement),"position < ordinal") != NULL);
    assert(text_is(rename_plan.target.definition,"position"));
    destroy_rename();
}
/** Prepare and revalidate hyphenated replacement names through the common
 * scanner and exact source edits, including international multiword spellings.
 */
static void it_renames_to_hyphenated_complete_names_without_changing_other_bytes(void) {
    const char *names[] = {"new-name", "new-name next-2", "ключ-2"};
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        project_fixture("@!\nvalue\n.cell\ncopy\n value\n");
        setup_rename();
        assert(prepare(".main", "value", names[index]));
        const EditorRenameFile *main = changed_file(".main");
        char expected[256];
        int written = snprintf(expected, sizeof(expected),
            "@!\n%s\n.cell\ncopy\n %s\n", names[index], names[index]);
        assert(written > 0 && (size_t)written < sizeof(expected));
        assert(main != NULL && strcmp((const char *)source_bytes(main->replacement), expected) == 0);
        assert(validate() && rename_plan.validated);
        destroy_rename();
    }
}

/** Check core source-edit construction with real catalog/parser/resolver/planner revalidation. */
void run(void) {
    it_renames_to_hyphenated_complete_names_without_changing_other_bytes();
    it_renames_callee_labels_in_unused_importers_without_changing_caller_values();
    it_renames_qualified_constants_across_files_and_preserves_unrelated_bytes();
    it_preserves_reference_list_suffixes_and_same_name_noops();
    it_preserves_literal_and_comment_bytes_with_real_standard_text_metadata();
    it_preserves_branch_loop_and_selected_method_refusal_graphs();
    it_preserves_intrinsic_labels_while_renaming_index_operands();
}
/** Every borrowed old snapshot and owned candidate/replacement prefix is released before success. */
int main(void) { run(); return EXIT_SUCCESS; }
