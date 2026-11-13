/**
 * @file
 * @brief Independently compile generated native views and reject const, nominal and private-storage violations.
 */
#include <errno.h>
#include "native/toolchain/profile.h"

/** Locate one exact declared fixture type, borrowing its checked scaffold lifetime. */
static const ScaffoldType *borrow_fixture_type(const char *path) {
    for (const ScaffoldType *type = scaffold.types; type != NULL; type = type->next) {
        if (strcmp(type->plan->type->module->source->logical_path, path) == 0) { return type; }
    }
    assert(false);
    return NULL;
}

/** Emit a typed positive client and four intentionally forbidden expressions behind exact case selectors. */
static void write_native_borrow_probe(void) {
    const ScaffoldType *value = borrow_fixture_type(".value");
    const ScaffoldType *twin = borrow_fixture_type(".twin");
    const ScaffoldType *node = borrow_fixture_type(".node");
    FILE *output = fopen("generated/borrow.c", "wb");
    assert(output != NULL);
    assert(fprintf(output,
        "#include \"%s\"\n"
        "void consume(const %s *view) { (void)view; }\n"
        "int main(void) {\n"
        "%s_State payload={0}; %s read={.payload=&payload}; %s write={.payload=&payload};\n"
        "%s twin={0}; %s reference={0}; %s list={0};\n"
        "(void)twin; (void)reference; (void)list; (void)write; consume(&read);\n"
        "#if AMA_VIEW_CASE == 1\n#line 1 \"AMA_EXPECTED_READ_ONLY_VIEW\"\nread.payload->number++;\n"
        "#elif AMA_VIEW_CASE == 2\n#line 1 \"AMA_EXPECTED_NOMINAL_VIEW\"\nconsume(&twin);\n"
        "#elif AMA_VIEW_CASE == 3\n#line 1 \"AMA_EXPECTED_OPAQUE_REFERENCE\"\n(void)reference.storage->location;\n"
        "#elif AMA_VIEW_CASE == 4\n#line 1 \"AMA_EXPECTED_OPAQUE_LIST\"\n(void)list.storage->location;\n"
        "#else\nwrite.payload->number=read.payload->number+1;\n#endif\nreturn 0; }\n",
        scaffold.selected->interface_name, value->names[SCAFFOLD_READ]->internal_name,
        value->native_prefix->internal_name, value->names[SCAFFOLD_READ]->internal_name,
        value->names[SCAFFOLD_WRITE]->internal_name, twin->names[SCAFFOLD_READ]->internal_name,
        node->names[SCAFFOLD_REFERENCE_READ]->internal_name, value->names[SCAFFOLD_LIST_READ]->internal_name) > 0);
    assert(fclose(output) == 0);
}

/** Require the supported C frontend to accept the baseline or diagnose the selected forbidden expression. */
static void compile_native_borrow_probe(unsigned selected, const char *diagnostic_location) {
    char selection[sizeof("-DAMA_VIEW_CASE=0")];
    int length = snprintf(selection, sizeof(selection), "-DAMA_VIEW_CASE=%u", selected);
    assert(length > 0 && (size_t)length < sizeof(selection));
    pid_t process = fork();
    assert(process >= 0);
    if (process == 0) {
        if (freopen("generated/borrow.log", "wb", stderr) == NULL) { _exit(126); }
        char *arguments[] = {TEST_REAL_COMPILER, AMA_TOOLCHAIN_REQUIRED_FLAGS,
            "-I" TEST_SDK, "-Igenerated/native", selection, "-fsyntax-only", "generated/borrow.c", NULL};
        execvp(arguments[0], arguments);
        _exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(process, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == process && WIFEXITED(status));
    if (WEXITSTATUS(status) == 0) { assert(diagnostic_location == NULL); return; }
    FILE *errors = fopen("generated/borrow.log", "rb");
    assert(errors != NULL);
    char text[8192];
    size_t count = fread(text, 1, sizeof(text) - 1, errors);
    assert(count < sizeof(text) - 1 && !ferror(errors) && fclose(errors) == 0);
    text[count] = '\0';
    if (diagnostic_location == NULL) { fputs(text, stderr); }
    assert(diagnostic_location != NULL && strstr(text, diagnostic_location) != NULL);
}

/** Native authors can write bound outputs but cannot cast nominally equal layouts or access private graph storage. */
static void it_enforces_native_view_constness_nominal_types_and_opaque_graph_storage(void) {
    prepare_native();
    file("generated/project/twin", "@C\n");
    file("generated/project/twin.h", "#ifndef TWIN_VIEW_TEST_H\n#define TWIN_VIEW_TEST_H\nstruct AMA_NATIVE(State) { size_t number; };\n#endif\n");
    file("generated/project/twin.c", "/* Syntax-only view client requires no twin body. */\n");
    file("generated/project/twin.abi", contract);
    file("generated/project/main", "@<\ninput\n.value\nother\n.twin\nlink&\n.node\nitems[]\n.value\n@>\noutput\n.value\n@C\n");
    begin_scaffold();
    assert(build_scaffold(false) && native_scaffold_publish(&scaffold));
    write_native_borrow_probe();
    compile_native_borrow_probe(0, NULL);
    const char *locations[] = {"AMA_EXPECTED_READ_ONLY_VIEW", "AMA_EXPECTED_NOMINAL_VIEW",
        "AMA_EXPECTED_OPAQUE_REFERENCE", "AMA_EXPECTED_OPAQUE_LIST"};
    for (unsigned selected = 1; selected <= 4; selected++) {
        compile_native_borrow_probe(selected, locations[selected - 1]);
    }
    end_scaffold();
}
