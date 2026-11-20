/** @file
 * @brief Reject invalid names, namespace collisions, silent successful-compilation captures and stale workspace versions.
 */
#include "../common.c"
/** Replacement validation consumes exactly one complete name and never strips prohibited syntax. */
static void it_rejects_invalid_complete_names_without_preparing_source_edits(void) {
    project_fixture("@!\nvalue\n/test/fixture/v1.0.0/src/cell\n");
    setup_rename();
    const char *invalid[] = {""," leading","trailing ","two  spaces","with\ttab","line\nbreak","field&","values[]","/test/fixture/v1.0.0/src/module","name#comment","1name","\xff",
        "-name", "name-", "name--part", "name- part", "name -part"};
    for (size_t index = 0; index < sizeof(invalid)/sizeof(*invalid); index++) {
        assert(!prepare("/test/fixture/v1.0.0/src/main","value",invalid[index]) && !rename_plan.prepared && rename_plan.first == NULL);
        assert(diagnostic_valid(&rename_plan.diagnostic));
        editor_rename_destroy(&rename_plan);
        editor_rename_init(&rename_plan,sources.allocator);
    }
    size_t offset = point(analysis.selected->text,"value");
    assert(!editor_rename_prepare(&rename_plan,&analysis,"/test/fixture/v1.0.0/src/main",offset,NULL,1));
    assert(rename_plan.diagnostic.code == DIAGNOSTIC_SYNTAX);
    editor_rename_destroy(&rename_plan);
    editor_rename_init(&rename_plan,sources.allocator);
    const unsigned char nul[] = {'n',0,'x'};
    assert(!editor_rename_prepare(&rename_plan,&analysis,"/test/fixture/v1.0.0/src/main",offset,nul,sizeof(nul)));
    assert(rename_plan.diagnostic.code == DIAGNOSTIC_SYNTAX && text_is(rename_plan.target.definition,"value"));
    destroy_rename();
}
/** A duplicate module namespace entry fails the actual compiler, not an editor spelling heuristic. */
static void it_rejects_callee_field_collisions_with_original_compiler_diagnostics(void) {
    project_fixture("@!\n");
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/worker","argument","result"));
    assert(!editor_rename_validate(&rename_plan) && !rename_plan.validated && !rename_plan.candidate.frontend.valid);
    assert(rename_plan.diagnostic.code == DIAGNOSTIC_TYPE_MISMATCH && diagnostic_valid(&rename_plan.diagnostic));
    destroy_rename();
}
/** Reusing an existing compatible reference can compile but changes a foreach local into another cell; rename must reject it. */
static void it_rejects_silent_capture_even_when_the_complete_candidate_compiles(void) {
    project_fixture("@!\nouter&\n/test/fixture/v1.0.0/src/cell\nvalues[]\n/test/fixture/v1.0.0/src/cell\nvalues[item&]\n{\n current&\n  item&\n}\n");
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/main","outer&","item"));
    assert(!editor_rename_validate(&rename_plan) && rename_plan.candidate.frontend.valid && rename_plan.candidate.indexed);
    assert(rename_plan.diagnostic.code == DIAGNOSTIC_RENAME_CONFLICT && !rename_plan.validated);
    editor_rename_destroy(&rename_plan);
    editor_rename_init(&rename_plan,sources.allocator);
    assert(prepare("/test/fixture/v1.0.0/src/main","item&","outer"));
    assert(!editor_rename_validate(&rename_plan) && rename_plan.candidate.frontend.valid);
    assert(rename_plan.diagnostic.code == DIAGNOSTIC_RENAME_CONFLICT);
    destroy_rename();
}
/** Disjoint local and later inferred names may legitimately coincide; unrelated declaration identity must remain separate. */
static void it_accepts_disjoint_foreach_names_without_renaming_a_later_same_spelled_field(void) {
    project_fixture("@!\nvalues[]\n/test/fixture/v1.0.0/src/cell\nworker\n/test/fixture/v1.0.0/src/worker\nvalues[item&]\n{\n alias&\n  item&\n}\nworker !\n result& > later&\n");
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/main","item&","later"));
    assert(validate());
    const EditorRenameFile *main = changed_file("/test/fixture/v1.0.0/src/main");
    assert(main != NULL && strstr((const char *)source_bytes(main->replacement),"values[later&]\n") != NULL);
    assert(strstr((const char *)source_bytes(main->replacement),"result& > later&\n") != NULL);
    destroy_rename();
}
/** An unrelated draft edit still invalidates the captured workspace version before a transaction can run. */
static void it_rejects_changed_source_inventory_and_bytes_before_candidate_application(void) {
    project_fixture("@!\nvalue\n/test/fixture/v1.0.0/src/cell\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/draft","incomplete draft\n");
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/main","value","renamed") && validate());
    file("generated/project/packages/test/fixture/v1.0.0/src/draft","different incomplete draft\n");
    assert(!editor_rename_validate(&rename_plan) && rename_plan.diagnostic.code == DIAGNOSTIC_CHANGED_INPUT);
    assert(!rename_plan.validated);
    destroy_rename();
    setup_rename();
    assert(prepare("/test/fixture/v1.0.0/src/main","value","renamed"));
    file("generated/project/packages/test/fixture/v1.0.0/src/added","@!\n");
    assert(!editor_rename_validate(&rename_plan) && rename_plan.diagnostic.code == DIAGNOSTIC_CHANGED_INPUT);
    destroy_rename();
}
/** Exercise actual compiler rejection, stricter identity preservation and byte-exact version guards independently. */
void run(void) {
    it_rejects_invalid_complete_names_without_preparing_source_edits();
    it_rejects_callee_field_collisions_with_original_compiler_diagnostics();
    it_rejects_silent_capture_even_when_the_complete_candidate_compiles();
    it_accepts_disjoint_foreach_names_without_renaming_a_later_same_spelled_field();
    it_rejects_changed_source_inventory_and_bytes_before_candidate_application();
}
/** No rejected plan may leak candidate scopes, input snapshots or replacement source owners. */
int main(void) { run(); return EXIT_SUCCESS; }
