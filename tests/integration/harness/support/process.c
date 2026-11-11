#ifndef AMA_TEST_HARNESS_PROCESS_C
#define AMA_TEST_HARNESS_PROCESS_C

static void execute_make(char *const arguments[]) {
    int descriptor = open("generated/make.log", O_WRONLY | O_CREAT | O_APPEND, 0600);
    struct rlimit limit = {0, 0};
    if (descriptor < 0 || dup2(descriptor, STDOUT_FILENO) < 0 ||
        dup2(descriptor, STDERR_FILENO) < 0 || setrlimit(RLIMIT_CORE, &limit) != 0 ||
        setenv("MAKEFLAGS", "", 1) != 0 || setenv("MFLAGS", "", 1) != 0) {
        _exit(127);
    }
    close(descriptor);
    execvp("make", arguments);
    _exit(127);
}

static int run_make(const char *directory, const char *target, const char *override) {
    char *arguments[] = {
        "make", "-s", "-j4", "-C", (char *) directory, (char *) target,
        "C_COMPILER=" TEST_REAL_COMPILER,
        "C_COMPILER_FLAGS=-O0 -std=c11 -DNDEBUG", "CPPFLAGS=", "LTO_FLAGS=",
        "LINKER_FLAGS=", "TEST_DEFINES=", (char *) override, NULL
    };
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execute_make(arguments);
    }
    int status;
    pid_t waited;
    do {
        waited = waitpid(process, &status, 0);
    } while (waited < 0 && errno == EINTR);
    assert(waited == process && WIFEXITED(status));
    return WEXITSTATUS(status);
}

#endif
