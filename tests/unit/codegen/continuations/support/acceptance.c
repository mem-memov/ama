/** @file
 * @brief Accept same-type suspension, independent copies and current-caller traces.
 */

/** Keep both same-type nodes in distinct stable slots while each yields to its
 * immediate caller. An explicit host resume advances only the saved root segment;
 * ordinary copying clears positions and explicit copying preserves independence.
 */
static void it_runs_same_type_nested_copies_under_an_explicit_resuming_host(void) {
    directory("generated/project");
    file("generated/project/node", "@<\nnext&\n.node\n@!\nprobe\n.worker\n"
        "next& !\n:\nprobe !\n < |1\n%\nprobe !\n < |2\n");
    host_launcher = "#include \"project.h\"\n#include \"project.c\"\n#include <assert.h>\n"
        "int main(void) { AmaProjectHost host={0};\n"
        "AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(result.status.code==AMA_OK && result.outcome==AMA_HOST_YIELDED);\n"
        "AmaHandle root=host.root;\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME);\n"
        "assert(result.status.code==AMA_OK && result.outcome==AMA_HOST_COMPLETED);\n"
        "assert(ama_handle_equal(root,host.root) && !host.disposed);\n"
        "AmaStatus status=ama_project_dispose(&host); assert(status.code==AMA_OK);\n"
        "return 0; }\n";
    execute_project("@!\nfirst\n.node\nsecond\n.node\ncopy\n.node\nplain\n.node\n"
        "probe\n.worker\nfirst !\n < &second\ncopy\n % first\nplain\n first\n%\n"
        "copy %\nfirst %\nsecond %\nplain %\n:\n probe !\n  < |99\n",
        "1:1:7\n1:1:0\n2:2:0\n2:2:0\n2:2:7\n99:1:10\n", true);
    host_launcher = NULL;
}

/** A copied saved statement must acquire a fresh return connection. Diagnose
 * the actual resumer, excluding the source's historical caller after failure.
 */
static void it_reports_only_the_current_caller_after_copying_a_continuation(void) {
    directory("generated/project");
    file("generated/project/task", "@!\nworker\n.worker\n%\nworker !\n < |500\n");
    file("generated/project/original", "@<\ntarget&\n.task\n@!\ntarget& !\n");
    file("generated/project/resumer", "@<\ntarget&\n.task\n@!\ntarget& %\n");
    execute_project("@!\ntask\n.task\ncopy\n.task\noriginal\n.original\nresumer\n.resumer\n"
        "original !\n < &task\ncopy\n % task\nresumer !\n < &copy\n", "native-entered\n", false);
    expect_diagnostic("generated/project/task:5:1");
    expect_diagnostic("generated/project/resumer:5:1");
    FILE *errors = fopen("generated/errors.txt", "rb");
    assert(errors != NULL);
    char bytes[4096] = {0};
    size_t count = fread(bytes, 1, sizeof(bytes) - 1, errors);
    int closed = fclose(errors);
    assert(count != 0 && closed == 0);
    assert(strstr(bytes, "generated/project/original:") == NULL);
}
