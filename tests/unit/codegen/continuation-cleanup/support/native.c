/** @file
 * @brief Preserve the synchronous native boundary instead of inventing native suspension or status-based yield.
 */
/** A native declaration cannot contain an Ama body yield. Its companions are
 * deliberately uncompilable: rejection must occur in frontend source analysis.
 */
static void it_rejects_ama_yield_inside_a_native_declaration(void) {
    directory("generated/project");
    file("generated/project/main", "@!\nvalue\n.native\nvalue !\n");
    file("generated/project/native", "@C\n%\n");
    file("generated/project/native.h", "#error Native yield must fail before C compilation\n");
    file("generated/project/native.c", "#error Native yield must fail before C compilation\n");
    file("generated/project/native.abi", "+ initialize\n- destroy\n");
    setup_frontend();
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    bool checked = compiler_frontend_check(&frontend, &root, 1, ".main");
    assert(!checked && !frontend.valid && !frontend.plan.valid && diagnostic_valid(&frontend.error));
    assert(frontend.error.primary.span.source != NULL);
    assert(strcmp(source_path(frontend.error.primary.span.source), "generated/project/native") == 0);
    destroy_frontend();
}

/** Return a valid nonzero continuation-related code from a completed native
 * activation. A native status cannot request a saved C-stack continuation.
 */
static bool return_native_status(FILE *output, const ScaffoldType *type) {
    if (strcmp(type->plan->type->module->source->logical_path, ".worker") != 0) { return false; }
    int written = fputs("puts(\"native-status\"); *status=ama_status_make(AMA_NO_RESUME,\"Native is synchronous\",sizeof(\"Native is synchronous\")-1); return;\n", output);
    assert(written != EOF);
    return true;
}

/** Native nonzero status is an admitted fatal execution result even when its
 * numeric code also appears in acquisition refusal. It never yields or resumes.
 */
static void it_keeps_native_status_failure_distinct_from_yield_and_admission(void) {
    native_entry_override = return_native_status;
    host_launcher = "#include \"project.h\"\n#include \"project.c\"\n#include <assert.h>\n"
        "int main(void) { AmaProjectHost host={0}; AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(ama_host_result_is_valid(result) && result.outcome==AMA_HOST_FAILED && result.status.code==AMA_NO_RESUME);\n"
        "assert(host.disposed && !host.registry.initialized && host.active.world==NULL);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.status.code==AMA_INVALID_STATE); return 0; }\n";
    execute_project("@!\nprobe\n.worker\nprobe !\n%\nprobe !\n < |999\n", "native-status\n", true);
    expect_diagnostic("Native is synchronous");
    host_launcher = NULL;
    native_entry_override = NULL;
}
