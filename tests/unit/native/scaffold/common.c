#ifndef AMA_SCAFFOLD_TEST_COMMON
#define AMA_SCAFFOLD_TEST_COMMON

#include "../../compiler/common.c"
#include <sys/wait.h>
#include <unistd.h>
#include "native/scaffold/scaffold.h"

static NativeScaffold scaffold;
static const char *contract = "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n";

static inline void prepare_native(void) {
    directory("generated/project");
    file("generated/project/main", "@<\n输入\n.value\nlink&\n.node\nitems[]\n.value\nlinks&[]\n.node\n@>\n输出\n.value\n@C\n");
    file("generated/project/value", "@C\n");
    file("generated/project/value.h", "#ifndef VALUE_TEST_H\n#define VALUE_TEST_H\nstruct AMA_NATIVE(State) { size_t number; };\n#endif\n");
    file("generated/project/value.c", "#include <stddef.h>\n_Static_assert(sizeof(size_t) > 0, \"Size type\");\n");
    file("generated/project/value.abi", contract);
    file("generated/project/node", "@!\nnext&\n.node\nvalue\n.value\n");
}

static void begin_scaffold(void) {
    setup_frontend();
    ModuleRoot root = {"generated/project", NULL, 0};
    bool valid = compiler_frontend_authoring(&frontend, &root, 1, ".main", (SemanticLibraries){0}, contract);
    assert(valid && frontend.authoring);
    native_scaffold_init(&scaffold, &frontend, sources.allocator);
}

static void end_scaffold(void) {
    native_scaffold_destroy(&scaffold);
    destroy_frontend();
}

static bool build_scaffold(bool overwrite) {
    return native_scaffold_build(&scaffold, (ScaffoldOptions){
        .directory = "generated/native", .sdk = TEST_SDK, .compiler = "gcc", .create = true, .overwrite = overwrite});
}

static inline int compile_file(const char *path) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execlp("gcc", "gcc", "-std=c11", "-pedantic-errors", "-Wall", "-Wextra", "-Werror",
            "-I" TEST_SDK, "-Igenerated/native", "-fsyntax-only", path, (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status));
    return WEXITSTATUS(status);
}


#endif
