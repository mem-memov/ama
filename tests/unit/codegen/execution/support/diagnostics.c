/** @file
 * @brief Check source-aware primary diagnostics and secondary destructor failures across the compiled project boundary.
 */
/** Cleanup may fail both after an active native call and while unwinding root construction; the original status must remain authoritative. */
static void it_preserves_primary_failure_when_native_cleanup_also_fails(void) {
    directory("generated/project");
    file("generated/project/outer", "@!\nchild\n.inner\nchild !\n");
    file("generated/project/inner", "@!\nworker\n.worker\nworker !\n < |500\n");
    fail_native_destroy = true;
    host_launcher = "#include <assert.h>\n#include <string.h>\n#include \"project.c\"\n"
        "int main(void) { AmaProjectHost host={0};\n"
        "AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(result.outcome==AMA_HOST_FAILED && result.status.code==AMA_NATIVE_FAILURE);\n"
        "assert(result.status.message_length==11 && memcmp(result.status.message,\"Body failed\",11)==0);\n"
        "AmaStatus status=ama_project_dispose(&host);\n"
        "assert(status.code==result.status.code && status.message==result.status.message);\n"
        "assert(host.disposed && host.active.world==NULL);\n"
        "assert(!host.registry.initialized);\n"
        "return EXIT_FAILURE; }\n";
    execute_project("@!\nouter\n.outer\nouter !\n", "native-entered\n", false);
    expect_diagnostic("Ama runtime failure: 4: Body failed");
    expect_diagnostic("Secondary Ama runtime failure: 4: cleanup");
    expect_diagnostic("generated/project/inner:4:1");
    expect_diagnostic("generated/project/outer:4:1");
    expect_diagnostic("generated/project/main:4:1");
    host_launcher = NULL;
    execute_project("@!\nworker\n.worker\nbad\n.value\n |bad\nworker !\n", "", false);
    expect_diagnostic("Not a decimal value");
    expect_diagnostic("Secondary Ama runtime failure: 4: cleanup");
    expect_diagnostic("type .value");
    fail_native_destroy = false;
}
