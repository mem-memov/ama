/** @file
 * @brief Compile actual lexical foreach storage from resolved source and plans.
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

/** Three real loops produce separate cursors; nested reuse shares one binding
 * cell while an existing persistent reference gets its own origin cell.
 */
static void it_compiles_fixed_cursors_and_one_cell_for_each_exact_binding(void) {
    directory("generated/project");
    directory("generated/standard");
    native_layout("generated/project", "value");
    native_layout("generated/standard", "integer");
    file("generated/project/main", "@!\nitems[]\n.value\nexisting&\n.value\nitems[first&]\n{\n items[first&]\n {\n  first& !\n }\n}\nitems[existing&]\n{\n existing& !\n}\n");
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
    bool emitted = codegen_emit_storage(&scaffold, output) && codegen_emit_descriptors(&scaffold, output);
    assert(emitted);
    written = fputs("#include \"runtime/lists/cursors/schema.c\"\n", output);
    assert(written != EOF);
    const ScaffoldType *root = scaffold.selected;
    written = fprintf(output, "int main(void) { %s_Storage value={0};\n", root->names[SCAFFOLD_WRITE]->internal_name);
    assert(written > 0);
    size_t cursors = 0, bindings = 0;
    for (const PlannedLoop *loop = root->plan->loops; loop != NULL; loop = loop->next) {
        assert(loop->statement->syntax->kind == AST_FOREACH);
        cursors++;
        written = fprintf(output, "_Static_assert(sizeof(value.foreach_cursor_%zu)==sizeof(AmaProgramForeachCursor),\"cursor\");"
            "assert(!value.foreach_cursor_%zu.active);\n", loop->entry->selector, loop->entry->selector);
        assert(written > 0);
        bool earlier = false;
        for (const PlannedLoop *prior = root->plan->loops; prior != loop; prior = prior->next) {
            if (prior->statement->traversal->binding == loop->statement->traversal->binding) { earlier = true; break; }
        }
        if (earlier) { continue; }
        bindings++;
        written = fprintf(output, "_Static_assert(sizeof(value.foreach_binding_%zu)==sizeof(AmaProgramForeachBinding),\"binding\");"
            "assert(value.foreach_binding_%zu.roles==0);\n", loop->entry->selector, loop->entry->selector);
        assert(written > 0);
    }
    assert(cursors == 3 && bindings == 2);
    const char *root_name = root->names[SCAFFOLD_WRITE]->internal_name;
    written = fprintf(output, "assert(ama_list_cursor_schema(sizeof(value),%zu,%s_ListCursors).code==AMA_OK);\n"
        "assert(sizeof(%s_Fields)/sizeof(%s_Fields[0])==%zu);\n",
        root->plan->field_count, root_name, root_name, root_name, root->plan->field_count);
    assert(written > 0);
    for (const ScaffoldMember *member = root->members; member != NULL; member = member->next) {
        size_t count = 0;
        for (const PlannedLoop *loop = root->plan->loops; loop != NULL; loop = loop->next) {
            if (loop->statement->traversal->collection != member->field->symbol) { continue; }
            written = fprintf(output, "assert(%s_ListCursors[%zu].offsets[%zu]==offsetof(%s_Storage,foreach_cursor_%zu));\n",
                root_name, member->field->slot, count++, root_name, loop->entry->selector);
            assert(written > 0);
        }
        written = fprintf(output, "assert(%s_ListCursors[%zu].count==%zu);\n", root_name, member->field->slot, count);
        assert(written > 0);
    }
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
void run(void) { it_compiles_fixed_cursors_and_one_cell_for_each_exact_binding(); }
/** Return success after the actual compiled layout probe and counted teardown. */
int main(void) { run(); return EXIT_SUCCESS; }
