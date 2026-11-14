/**
 * @file
 * @brief Observe native borrow closure in real generated execution before terminal disposal or host return.
 */

/** Checked host observations remain outside native code; no borrowed Ama address is retained in payloads. */
static void it_closes_native_borrows_before_normal_or_failed_host_return(void) {
    host_launcher = "#include <assert.h>\n#include \"project.h\"\n#include \"project.c\"\n"
        "int main(void) {\n"
        "AmaProjectHost host={0}; AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(host.active.world==NULL); if(!host.disposed) { AmaContextLease lease={0}; AmaContextView view={0};\n"
        "assert(ama_context_enter(&host.registry,host.identity,&lease).code==AMA_OK);\n"
        "assert(ama_context_inspect(&lease,&view).code==AMA_OK && !view.world->operation_active);\n"
        "assert(ama_context_leave(&lease).code==AMA_OK); }\n"
        "assert((result.status.code!=AMA_OK)==host.disposed);\n"
        "AmaStatus status=ama_project_dispose(&host);\n"
        "assert(host.active.world==NULL && host.disposed);\n"
        "assert(!host.registry.initialized);\n"
        "return status.code==AMA_OK ? EXIT_SUCCESS : EXIT_FAILURE; }\n";
    execute_project("@!\nworker\n.worker\nworker !\n", "7:1:0\n", true);
    malformed_native_entry = true;
    execute_project("@!\nworker\n.worker\nworker !\n", "7:1:0\n", false);
    malformed_native_entry = false;
    host_launcher = NULL;
    expect_diagnostic("Malformed native status");
}
