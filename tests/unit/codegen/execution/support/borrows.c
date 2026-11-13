/**
 * @file
 * @brief Observe native borrow closure in real generated execution before terminal disposal or host return.
 */

/** Checked host observations remain outside native code; no borrowed Ama address is retained in payloads. */
static void it_closes_native_borrows_before_normal_or_failed_host_return(void) {
    host_launcher = "#include <assert.h>\n#include \"project.h\"\n#include \"project.c\"\n"
        "int main(void) {\n"
        "AmaProjectHost host={0}; AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(!host.world.operation_active);\n"
        "assert((result.status.code!=AMA_OK)==host.disposed);\n"
        "AmaStatus status=ama_project_dispose(&host);\n"
        "assert(!host.world.operation_active && host.disposed);\n"
        "for(size_t index=0;index<host.world.type_count;index++) assert(host.world.types[index]->pool.live_count==0);\n"
        "return status.code==AMA_OK ? EXIT_SUCCESS : EXIT_FAILURE; }\n";
    execute_project("@!\nworker\n.worker\nworker !\n", "7:1:0\n", true);
    malformed_native_entry = true;
    execute_project("@!\nworker\n.worker\nworker !\n", "7:1:0\n", false);
    malformed_native_entry = false;
    host_launcher = NULL;
    expect_diagnostic("Malformed native status");
}
