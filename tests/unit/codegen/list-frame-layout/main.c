/** @file
 * @brief Compile actual lexical list frame layout from resolved source and plans.
 */
#include "../../compiler/common.c"
#include "codegen/internal.h"
#include <sys/wait.h>
#include <unistd.h>

/** Author a native fixture whose complete State is included in the layout probe.
 * Hook bodies are not executed by this representation-only component boundary.
 */
static void native_layout(const char *directory_path, const char *name) {
    char path[160];
    int length = snprintf(path, sizeof(path), "%s/%s", directory_path, name);
    assert(length > 0 && (size_t)length < sizeof(path));
    file(path, "@C\n");
    length = snprintf(path, sizeof(path), "%s/%s.h", directory_path, name);
    assert(length > 0);
    file(path, "struct AMA_NATIVE(State) { size_t number; };\n");
    length = snprintf(path, sizeof(path), "%s/%s.c", directory_path, name);
    assert(length > 0);
    file(path, "/* This leaf compiles storage, not native entry implementations. */\n");
    length = snprintf(path, sizeof(path), "%s/%s.abi", directory_path, name);
    assert(length > 0);
    file(path, "+ initialize\n- destroy\n| literal\n= clone\n[] index\n");
}

/** Require normal child completion, never accepting a signal as test success. */
static void checked_process(pid_t process) {
    int status = 0;
    pid_t waited = waitpid(process, &status, 0);
    assert(waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}

/** Real semantic frames produce individually typed stable C cells, with no
 * hidden method instance and no list-frame allocation for unrelated call sites.
 */
static void it_compiles_typed_lexical_list_service_and_method_storage(void) {
    directory("generated/project");
    directory("generated/standard");
    native_layout("generated/project", "value");
    native_layout("generated/standard", "integer");
    file("generated/project/main", "@!\nitems[]\n.value\nitems[]\n clear !\n clear !\n");
    setup_frontend();
    ModuleRoot roots[] = {{"generated/project", NULL, 0, MODULE_ROOT_PROJECT},
        {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    bool checked = compiler_frontend_check(&frontend, roots, 2, ".main");
    assert(checked && frontend.plan.valid);
    NativeScaffold scaffold;
    native_scaffold_init(&scaffold, &frontend, sources.allocator);
    scaffold.options = (ScaffoldOptions){.directory = "generated/output", .sdk = TEST_SDK};
    bool prepared = scaffold_paths(&scaffold) && scaffold_model(&scaffold);
    assert(prepared);
    FILE *output = fopen("generated/probe.c", "wb");
    assert(output != NULL);
    int written = fputs("#include <stddef.h>\n#include <assert.h>\n#include \"runtime/program/calls/call.h\"\n", output);
    assert(written != EOF);
    for (const ScaffoldType *type = scaffold.types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) { continue; }
        const char *logical = type->plan->type->module->source->logical_path;
        assert(strcmp(logical, ".value") == 0 || strcmp(logical, "..integer") == 0);
        const char *header = strcmp(logical, ".value") == 0 ? "project/value.h" : "standard/integer.h";
        written = fprintf(output, "#define AMA_NATIVE(name) %s_##name\n#include \"%s\"\n"
            "typedef struct %s_State %s_State;\n#undef AMA_NATIVE\n", type->native_prefix->internal_name,
            header, type->native_prefix->internal_name, type->native_prefix->internal_name);
        assert(written > 0);
    }
    bool emitted = codegen_emit_storage(&scaffold, output);
    assert(emitted);
    const ScaffoldType *root = scaffold.selected;
    written = fprintf(output, "int main(void) { %s_Storage value={0};\n", root->names[SCAFFOLD_WRITE]->internal_name);
    assert(written > 0);
    size_t services = 0, methods = 0;
    for (const StorageFrame *frame = root->plan->frames; frame != NULL; frame = frame->next) {
        if (frame->statement->operation_kind == SEMANTIC_LIST_SERVICE_OPERATION) {
            services++;
            written = fprintf(output, "_Static_assert(sizeof(value.list_service_%zu)==sizeof(AmaProgramListService),\"service\");"
                "assert(!value.list_service_%zu.active);\n", frame->slot, frame->slot);
        } else if (frame->statement->operation_kind == SEMANTIC_LIST_METHOD_OPERATION) {
            methods++;
            written = fprintf(output, "_Static_assert(sizeof(value.list_method_%zu)==sizeof(AmaProgramListMethod),\"method\");"
                "assert(!value.list_method_%zu.active);\n", frame->slot, frame->slot);
        } else { assert(false); }
        assert(written > 0);
    }
    assert(services == 1 && methods == 2);
    written = fputs("return 0; }\n", output);
    assert(written != EOF);
    int closed = fclose(output);
    assert(closed == 0);
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        execlp("gcc", "gcc", "-std=c11", "-pedantic", "-Wall", "-Wextra", "-Werror", "-UNDEBUG",
            "-I" TEST_SDK, "generated/probe.c", "-o", "generated/probe", (char *)NULL);
        _exit(127);
    }
    checked_process(process);
    process = fork();
    assert(process >= 0);
    if (process == 0) { execl("generated/probe", "generated/probe", (char *)NULL); _exit(127); }
    checked_process(process);
    native_scaffold_destroy(&scaffold);
    destroy_frontend();
}

/** Dispatch the resolved-plan/actual-C storage handoff explicitly. */
void run(void) { it_compiles_typed_lexical_list_service_and_method_storage(); }
/** Return success after the actual compiled layout probe and counted teardown. */
int main(void) { run(); return EXIT_SUCCESS; }
