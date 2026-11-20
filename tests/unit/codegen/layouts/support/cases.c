/** @file
 * @brief Compose source-to-plan emission with actual compiled C layouts and reject same-representation nominal substitutions before emission.
 */
/** Real generated/native C agrees with the plan's forms/identities and target C offsets/alignment throughout lifecycle, pool growth and borrowed views. */
static void it_composes_planned_fields_with_actual_c_layout_lifecycle_and_native_borrows(void) {
    prepare_layout();
    begin_layout(true);
    bool built = project_codegen_build(&assembly, (ScaffoldOptions){.directory = "generated/output", .sdk = TEST_SDK});
    assert(built);
    for (const ScaffoldType *type = assembly.types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE) { write_layout_native(type); }
    }
    bool compiled = project_codegen_compile(&assembly, stderr);
    assert(compiled);
    bool published = native_scaffold_publish(&assembly);
    assert(published);
    FILE *output = fopen("generated/layout-probe.c", "wb");
    assert(output != NULL);
    int written = fputs("#include <assert.h>\n#include <stdint.h>\n#include <stdio.h>\n#include \"project.h\"\n#include \"project.c\"\n", output);
    assert(written != EOF);
    written = fputs("static AmaContextView test_view;\nstatic void check_status(AmaStatus status) { assert(status.code==AMA_OK); (void)status; }\n", output);
    assert(written != EOF);
    probe_aliases(output);
    probe_layout_checks(output);
    probe_value_checks(output);
    probe_borrow_checks(output);
    written = fputs(
        "int main(void) { AmaProjectHost host={0}; AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(result.outcome==AMA_HOST_COMPLETED && result.status.code==AMA_OK);\n"
        "AmaContextLease lease={0}; check_status(ama_context_enter(&host.registry,host.identity,&lease));\n"
        "check_status(ama_context_inspect(&lease,&test_view));\n"
        "check_layouts(&host); ROOT_STORAGE *root=check_values(&host); check_borrows(&host,root);\n"
        "printf(\"C layout: State=%zu align=%zu plain=%zu worker=%zu root=%zu\\n\",sizeof(PLAIN_STATE),_Alignof(PLAIN_STATE),sizeof(PLAIN_STORAGE),sizeof(WORKER_STORAGE),sizeof(ROOT_STORAGE));\n"
        "check_status(ama_context_leave(&lease)); test_view=(AmaContextView){0};\n"
        "check_status(ama_project_dispose(&host));\n"
        "assert(PLAIN_DEFAULTS+PLAIN_LITERALS+PLAIN_CLONES==PLAIN_DESTROYS);\n"
        "assert(TWIN_DEFAULTS+TWIN_LITERALS+TWIN_CLONES==TWIN_DESTROYS);\n"
        "assert(WORKER_DEFAULTS+WORKER_LITERALS+WORKER_CLONES==WORKER_DESTROYS);\n"
        "assert(host.active.world==NULL && !host.registry.initialized); return 0; }\n", output);
    assert(written != EOF);
    int closed = fclose(output);
    assert(closed == 0);
    compile_layout_probe();
    run_layout_probe();
    native_scaffold_destroy(&assembly);
    destroy_frontend();
}
/** Equal State size/alignment does not authorize owned/reference/list assignment between different module definitions. */
static void it_rejects_equal_native_layouts_as_substitutes_for_nominal_module_identity(void) {
    const char *forms[] = {"", "&", "[]", "&[]"};
    for (size_t index = 0; index < 4; index++) {
        prepare_layout();
        char source[512];
        int length = snprintf(source, sizeof(source), "@!\nfirst%s\n/test/fixture/v1.0.0/src/plain\nother%s\n/test/fixture/v1.0.0/src/twin\nfirst%s\n other%s\n",
            forms[index], forms[index], forms[index], forms[index]);
        assert(length > 0 && (size_t)length < sizeof(source));
        file("generated/project/packages/test/fixture/v1.0.0/src/main", source);
        begin_layout(false);
        assert(frontend.specialization_initialized && !frontend.specializations.valid && !frontend.semantic_initialized);
        assert(frontend.specializations.constraints.status == STATIC_CONSTRAINT_CONFLICT);
        assert(!frontend.plan_initialized && diagnostic_valid(&frontend.error));
        assert(frontend.error.primary.span.source == frontend.project.entry->source->text);
        destroy_frontend();
    }
}
