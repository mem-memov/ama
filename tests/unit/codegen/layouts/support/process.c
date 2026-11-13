/** @file
 * @brief Compile and execute a standalone actual-C layout/borrow probe, optionally instrumented by address/undefined-behavior sanitizers.
 */
/** Check child completion as an ordinary exited process; compiler/assertion failure is never accepted as successful evidence. */
static void wait_layout_process(pid_t process) {
    int status = 0;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Compile the written probe against the real published project and runtime SDK, with no reads of generated implementation text. */
static void compile_layout_probe(void) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        if (getenv("AMA_LAYOUT_SANITIZE") != NULL) {
            execlp("gcc", "gcc", "-std=c11", "-pedantic", "-Wall", "-Wextra", "-Werror", "-UNDEBUG", "-O0", "-g",
                "-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-I" TEST_SDK, "-Igenerated/output",
                "generated/layout-probe.c", "-o", "generated/layout-probe", (char *)NULL);
        } else {
            execlp("gcc", "gcc", "-std=c11", "-pedantic", "-Wall", "-Wextra", "-Werror", "-UNDEBUG", "-O0",
                "-I" TEST_SDK, "-Igenerated/output", "generated/layout-probe.c", "-o", "generated/layout-probe", (char *)NULL);
        }
        _exit(127);
    }
    wait_layout_process(process);
}
/** Execute the probe's native/runtime assertions and report measured C sizes to the leaf log. */
static void run_layout_probe(void) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execl("generated/layout-probe", "generated/layout-probe", (char *)NULL);
        _exit(127);
    }
    wait_layout_process(process);
}
