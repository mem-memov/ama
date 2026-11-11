#include "../../compiler/common.c"
#include <stdint.h>
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

static void emit_executable_native(const ScaffoldType *type) {
    FILE *body = fopen(type->body_path, "wb");
    assert(body != NULL);
    bool emitted = scaffold_include(body, type->interface_name) && scaffold_include(body, type->begin_name);
    assert(emitted);
    int written = fprintf(body, "AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const %s *parameters, %s *results, const AmaNativeServices *services, AmaStatus *status) { (void)state; (void)parameters; (void)results; (void)services; *status=ama_status_success(); }\n",
        type->names[SCAFFOLD_PARAMETERS]->internal_name, type->names[SCAFFOLD_RESULTS]->internal_name);
    assert(written >= 0);
    written = fputs("#include <assert.h>\n", body);
    assert(written != EOF);
    for (AbiOperation operation = ABI_DEFAULT; operation < ABI_BINDING_COUNT; operation++) {
        const AbiBinding *binding = &type->plan->type->abi.bindings[operation];
        emitted = scaffold_emit_signature(body, type, operation, binding->function);
        assert(emitted);
        const char *implementation = operation == ABI_DEFAULT ? " { destination->number=1; return ama_status_success(); }\n"
            : operation == ABI_DESTROY ? " { assert(state->number != 0); printf(\"%zu \", state->number); return ama_status_success(); }\n"
            : operation == ABI_LITERAL ? " { destination->number=literal.length; return ama_status_success(); }\n"
            : operation == ABI_CLONE ? " { destination->number=source->number; return ama_status_success(); }\n"
            : operation == ABI_CONDITION ? " { *result=state->number != 0; return ama_status_success(); }\n"
            : " { *result=state->number; return ama_status_success(); }\n";
        written = fputs(implementation, body);
        assert(written != EOF);
    }
    emitted = scaffold_include(body, type->end_name);
    int closed = fclose(body);
    assert(emitted && closed == 0);
}

static void it_runs_constant_and_root_lifetimes_from_the_emitted_heap_layout(void) {
    begin_assembly();
    bool built = build_assembly();
    assert(built);
    for (const ScaffoldType *type = assembly.types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE) { emit_executable_native(type); }
    }
    bool published = native_scaffold_publish(&assembly);
    assert(published);
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execlp("gcc", "gcc", "-std=c11", "-O2", "-pedantic", "-Wall", "-Wextra", "-Werror",
            "-I" TEST_SDK, "-Igenerated/output", "generated/output/launcher.c", "-o", "generated/program", (char *)NULL);
        _exit(127);
    }
    int status;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    int channel[2];
    int opened = pipe(channel);
    assert(opened == 0);
    process = fork();
    assert(process >= 0);
    if (process == 0) {
        close(channel[0]);
        if (dup2(channel[1], STDOUT_FILENO) < 0) { _exit(126); }
        close(channel[1]);
        execl("generated/program", "generated/program", (char *)NULL);
        _exit(127);
    }
    close(channel[1]);
    char bytes[100] = {0};
    size_t count = 0;
    ssize_t received;
    while ((received = read(channel[0], bytes + count, sizeof(bytes) - count - 1)) > 0) { count += (size_t)received; }
    close(channel[0]);
    waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    assert(strcmp(bytes, "1 1 12 ") == 0);
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

static void it_refuses_source_map_identity_exhaustion_without_wrapping_to_zero(void) {
    begin_assembly();
    frontend.plan.next_selector = SIZE_MAX;
    bool built = build_assembly();
    assert(!built && !assembly.ready && assembly.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
    end_assembly();
}

void run(void) {
    it_composes_the_real_runtime_and_qualified_native_sources_in_one_c_unit();
    it_runs_constant_and_root_lifetimes_from_the_emitted_heap_layout();
    it_unwinds_every_assembly_emission_allocation_failure();
    it_refuses_source_map_identity_exhaustion_without_wrapping_to_zero();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
