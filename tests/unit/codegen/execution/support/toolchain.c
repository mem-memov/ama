/**
 * @file
 * @brief Verify mandatory compile options and forbidden Ama functions in the real project pipeline.
 *
 * Included after execution fixtures. Compile/reject decisions use maintained native
 * companions, actual GCC/nm and the production publication gate; no binary is inspected here.
 */

/** Borrow one authored native fixture path for deliberate toolchain-boundary violations. */
static const ScaffoldType *toolchain_native_fixture(void) {
    for (const ScaffoldType *type = assembly.types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE
            && strncmp(source_path(type->plan->type->module->source->text),
                "generated/project/", sizeof("generated/project/") - 1) == 0) {
            return type;
        }
    }
    assert(false);
    return NULL;
}

/** Real compilation and an independent JSON consumer must share the enforced profile.
 * User options cannot weaken C11, mandatory diagnostics or non-LTO object inspection.
 */
static void it_uses_the_required_profile_in_real_compilation_and_the_compilation_database(void) {
    weak_toolchain_options = true;
    execute_project("@!\nfirst\n.worker\nfirst!\n", "7:1:0\n", true);
    weak_toolchain_options = false;
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execlp("python3", "python3", "-c",
            "import json,subprocess; row=json.load(open('generated/output/compile_commands.json'))[0]; "
            "a=row['arguments']; "
            "required=['-std=c11','-pedantic-errors','-Wall','-Wextra','-Werror',"
            "'-Werror=attributes','-fno-lto','-fno-whole-program']; "
            "assert all(a.count(x)==1 for x in required); "
            "assert [a.index(x) for x in required]==sorted(a.index(x) for x in required); "
            "assert a.index('-std=c99')<a.index('-std=c11'); "
            "assert a.index('-Wno-error')<a.index('-Werror'); "
            "assert a.index('-flto')<a.index('-fno-lto'); "
            "subprocess.run(a+['-o','generated/profile.o'],cwd=row['directory'],check=True)",
            (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/** A warning still fails real compilation when user flags requested -Wno-error.
 * Failure cannot retain a new executable or open the publication gate.
 */
static void it_rejects_native_warnings_despite_weaker_user_options(void) {
    weak_toolchain_options = true;
    prepare_project("@!\nfirst\n.worker\nfirst!\n");
    weak_toolchain_options = false;
    FILE *body = fopen(toolchain_native_fixture()->body_path, "ab");
    assert(body != NULL);
    int written = fputs("\nstatic void ama_test_unused_function(void) {}\n", body);
    assert(written != EOF && fclose(body) == 0);
    FILE *errors = tmpfile();
    assert(errors != NULL);
    bool compiled = project_codegen_compile(&assembly, errors);
    rewind(errors);
    char text[8192];
    size_t length = fread(text, 1, sizeof(text) - 1, errors);
    assert(!ferror(errors) && length < sizeof(text) - 1 && fclose(errors) == 0);
    text[length] = '\0';
    assert(!compiled && !assembly.ready && strstr(text, "unused-function") != NULL);
    assert(strstr(text, "error:") != NULL);
    assert(!native_scaffold_publish(&assembly));
    native_scaffold_destroy(&assembly);
    destroy_frontend();
}

/** The production verifier must reject an exact IR control name as a C function.
 * A separate C label with the same spelling is legal C, so only object verification
 * establishes that the forbidden function did not cross the publication boundary.
 */
static void it_rejects_an_ama_control_function_before_linking_or_publication(void) {
    prepare_project("@!\nfirst\n.worker\nfirst!\n");
    FILE *body = fopen(toolchain_native_fixture()->body_path, "ab");
    assert(body != NULL);
    int written = fprintf(body, "\n__attribute__((noinline,noipa)) void ama_label_%zu(void) {}\n",
        assembly.selected->plan->entry->selector);
    assert(written >= 0 && fclose(body) == 0);
    FILE *errors = tmpfile();
    assert(errors != NULL);
    bool compiled = project_codegen_compile(&assembly, errors);
    assert(fclose(errors) == 0 && !compiled && !assembly.ready);
    assert(strstr(assembly.error.message, "Ama control entry") != NULL);
    for (const ScaffoldArtifact *artifact = assembly.first; artifact != NULL; artifact = artifact->next) {
        assert(!artifact->executable);
    }
    assert(!native_scaffold_publish(&assembly));
    native_scaffold_destroy(&assembly);
    destroy_frontend();
}
