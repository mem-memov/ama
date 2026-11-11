#include "../../compiler/common.c"
#include <sys/wait.h>
#include <unistd.h>
#include "native/scaffold/scaffold.h"

static NativeScaffold scaffold;
static const char *contract = "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n";

static void prepare_native(void) {
    directory("generated/project");
    file("generated/project/main", "@<\n输入\n.value\nlink&\n.node\nitems[]\n.value\nlinks&[]\n.node\n@>\n输出\n.value\n@C\n");
    file("generated/project/value", "@C\n");
    file("generated/project/value.h", "#ifndef VALUE_TEST_H\n#define VALUE_TEST_H\nstruct AMA_NATIVE(State) { size_t number; };\n#endif\n");
    file("generated/project/value.c", "/* The selected template does not include companion bodies. */\n");
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

static int compile_file(const char *path) {
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

static void it_generates_complete_cycle_safe_headers_and_standalone_native_scaffolds(void) {
    prepare_native();
    begin_scaffold();
    bool built = build_scaffold(false);
    assert(built);
    bool published = native_scaffold_publish(&scaffold);
    assert(published);
    char probe[4096];
    int length = snprintf(probe, sizeof(probe),
        "#include \"%s\"\n#include \"%s\"\n"
        "#if defined(AMA_NATIVE) || defined(AMA_INPUT) || defined(AMA_OUTPUT) || defined(AMA_FIELD)\n#error Leaked namespace\n#endif\n"
        "int main(void) { return 0; }\n", scaffold.selected->interface_name, scaffold.selected->interface_name);
    assert(length > 0 && (size_t)length < sizeof(probe));
    file("generated/probe.c", probe);
    int compiled = compile_file("generated/probe.c");
    assert(compiled == 0);
    compiled = compile_file("generated/project/main.c");
    assert(compiled == 0);
    end_scaffold();
}

static void it_exports_a_database_that_an_independent_json_consumer_can_execute(void) {
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execlp("python3", "python3", "-c",
            "import json,subprocess; "
            "db=json.load(open('generated/native/compile_commands.json')); "
            "assert len(db)==4; "
            "[subprocess.run(row['arguments'],cwd=row['directory'],check=True) for row in db]",
            (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

static void it_unwinds_each_scaffold_model_allocation_failure(void) {
    begin_scaffold();
    size_t start = allocation_count;
    bool built = build_scaffold(false);
    assert(built);
    size_t count = allocation_count - start;
    end_scaffold();
    for (size_t failure = 1; failure <= count; failure++) {
        begin_scaffold();
        failure_at = allocation_count + failure;
        built = build_scaffold(false);
        assert(!built && scaffold.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        end_scaffold();
    }
}

void run(void) {
    it_generates_complete_cycle_safe_headers_and_standalone_native_scaffolds();
    it_exports_a_database_that_an_independent_json_consumer_can_execute();
    it_unwinds_each_scaffold_model_allocation_failure();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
