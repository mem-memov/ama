/** @file
 * @brief Change a yielded module's exposed predicate and resume its already chosen branch.
 */
/** This native flag has an explicit condition hook and a typed setter input;
 * updating it through its declared returned reference performs an ordinary call.
 */
static bool flag_entry(FILE *output, const ScaffoldType *type) {
    if (strcmp(type->plan->type->module->source->logical_path, ".flag") != 0) { return false; }
    int written = fputs("state->number=parameters->AMA_INPUT(0)->payload->number;\n", output);
    assert(written != EOF);
    return true;
}

/** Resume starts after the yield inside the positive branch. The now-false
 * predicate must not cause entry replay, rechecking, or the alternative branch.
 */
static void it_resumes_the_saved_branch_statement_after_external_condition_change(void) {
    directory("generated/project");
    native_fixture("flag", "@<\ninput\n.value\n@C\n");
    file("generated/project/task", "@>\ncontrol&\n.flag\n@!\nflag\n.flag\nprobe\n.worker\nflag !\n < |1\ncontrol&\n &flag\n"
        "flag\n?\n probe !\n  < |1\n %\n probe !\n  < |2\n:\n probe !\n  < |999\nprobe !\n < |3\n");
    native_entry_override = flag_entry;
    execute_project("@!\ntask\n.task\ntask !\n > control&\ncontrol& !\n < |0\ntask %\n", "1:1:0\n2:2:0\n3:3:0\n", true);
    native_entry_override = NULL;
}
