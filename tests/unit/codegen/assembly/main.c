#include "../../compiler/common.c"
#include <sys/wait.h>
#include <unistd.h>
#include "codegen/codegen.h"
#include "native/scaffold/internal.h"

static NativeScaffold assembly;

static void prepare_assembly(void) {
    directory("generated/project");
    file("generated/project/main", "@=\n常量\n.value\n |native bytes\n@!\nfirst\n.value\nsecond\n.other\nreference&\n.node\n");
    const char *names[] = {"value", "other"};
    for (size_t index = 0; index < 2; index++) {
        char path[128];
        int length = snprintf(path, sizeof(path), "generated/project/%s", names[index]);
        assert(length > 0);
        file(path, "@C\n");
        length = snprintf(path, sizeof(path), "generated/project/%s.h", names[index]);
        assert(length > 0);
        file(path, index == 0 ? "#ifndef VALUE_H\n#define VALUE_H\nstruct AMA_NATIVE(State) { size_t number; };\n#endif\n"
            : "#ifndef OTHER_H\n#define OTHER_H\nstruct AMA_NATIVE(State) { size_t number; };\n#endif\n");
        length = snprintf(path, sizeof(path), "generated/project/%s.c", names[index]);
        assert(length > 0);
        file(path, "/* Replaced by the test's handwritten native definition. */\n");
        length = snprintf(path, sizeof(path), "generated/project/%s.abi", names[index]);
        assert(length > 0);
        file(path, "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n");
    }
    file("generated/project/node", "@!\nnext&\n.node\n");
}

static void begin_assembly(void) {
    setup_frontend();
    ModuleRoot root = {"generated/project", NULL, 0};
    bool checked = compiler_frontend_check(&frontend, &root, 1, ".main", (SemanticLibraries){0});
    assert(checked);
    native_scaffold_init(&assembly, &frontend, sources.allocator);
}

static bool build_assembly(void) {
    return project_codegen_build(&assembly, (ScaffoldOptions){.directory = "generated/output", .sdk = TEST_SDK});
}

static void end_assembly(void) {
    native_scaffold_destroy(&assembly);
    destroy_frontend();
}

static void it_composes_the_real_runtime_and_qualified_native_sources_in_one_c_unit(void) {
    prepare_assembly();
    begin_assembly();
    bool built = build_assembly();
    assert(built);
    for (const ScaffoldType *type = assembly.types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) { continue; }
        FILE *body = fopen(type->body_path, "wb");
        assert(body != NULL);
        bool emitted = scaffold_emit_body(&assembly, body, type);
        int closed = fclose(body);
        assert(emitted && closed == 0);
    }
    bool published = native_scaffold_publish(&assembly);
    assert(published);
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execlp("gcc", "gcc", "-std=c11", "-pedantic", "-Wall", "-Wextra", "-Werror",
            "-I" TEST_SDK, "-Igenerated/output", "-fsyntax-only", "generated/output/launcher.c", (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    end_assembly();
}

static void it_unwinds_every_assembly_emission_allocation_failure(void) {
    begin_assembly();
    size_t start = allocation_count;
    bool built = build_assembly();
    assert(built);
    size_t count = allocation_count - start;
    end_assembly();
    for (size_t failure = 1; failure <= count; failure++) {
        begin_assembly();
        failure_at = allocation_count + failure;
        built = build_assembly();
        assert(!built && assembly.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        end_assembly();
    }
}

void run(void) {
    it_composes_the_real_runtime_and_qualified_native_sources_in_one_c_unit();
    it_unwinds_every_assembly_emission_allocation_failure();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
