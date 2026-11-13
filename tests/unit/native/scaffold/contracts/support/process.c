/** @file
 * @brief Compile self-contained emitted headers and current-source native probes under strict C, preserving explicit failure expectations.
 */
/** Require a normally exited compiler/program and return its observed process status. */
static int wait_contract(pid_t process) {
    int status = 0;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status));
    return WEXITSTATUS(status);
}
/** Compile a caller-written source against actual generated headers; expected invalid C is diagnosed by the real compiler, not text parsing. */
static void compile_contract(const char *source, bool syntax, bool success) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        if (!success && freopen("generated/contract-errors.txt", "wb", stderr) == NULL) { _exit(126); }
        if (syntax) {
            execlp("gcc", "gcc", "-std=c11", "-pedantic", "-Wall", "-Wextra", "-Werror", "-UNDEBUG",
                "-I", TEST_SDK, "-I", scaffold.directory, "-fsyntax-only", source, (char *)NULL);
        } else {
            execlp("gcc", "gcc", "-std=c11", "-pedantic", "-Wall", "-Wextra", "-Werror", "-UNDEBUG",
                "-I", TEST_SDK, "-I", scaffold.directory, source, "-o", "generated/contract-probe", (char *)NULL);
        }
        _exit(127);
    }
    int code = wait_contract(process);
    assert((code == 0) == success);
}
/** Execute the just-compiled matching header/body or fixed-bridge checks. */
static void run_contract(void) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) { execl("generated/contract-probe", "generated/contract-probe", (char *)NULL); _exit(127); }
    int code = wait_contract(process);
    assert(code == 0);
}
/** Each interface is its own first include, and all temporary provider/name/field aliases must end before unrelated C code. */
static void check_contract_headers(void) {
    for (const ScaffoldType *type = scaffold.types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) { continue; }
        FILE *output = fopen("generated/header-probe.c", "wb");
        assert(output != NULL);
        bool included = scaffold_include(output, type->interface_name);
        assert(included);
        int written = fputs("#if defined(AMA_NATIVE) || defined(AMA_NATIVE_ERROR) || defined(AMA_FIELD) || defined(AMA_INPUT) || defined(AMA_OUTPUT)\n#error Native context leaked\n#endif\n", output);
        int closed = fclose(output);
        assert(written != EOF && closed == 0);
        compile_contract("generated/header-probe.c", true, true);
    }
}
