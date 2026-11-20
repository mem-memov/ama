/** @file
 * @brief Observe explicit generated-host outcomes, nonmutating admission rejection and terminal failure.
 */
/** Admission responses are checked before interpreting accepted execution
 * outcomes. This internal generated host is not the public checked component ABI.
 */
static void it_preserves_healthy_hosts_across_resume_rejection_and_explicit_lifecycle(void) {
    host_launcher = "#include \"project.h\"\n#include \"project.c\"\n#include <assert.h>\n"
        "int main(void) { AmaProjectHost host={0};\n"
        "AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_RESUME);\n"
        "assert(result.status.code==AMA_NO_RESUME && !host.initialized && !host.disposed && host.identity==0);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(ama_host_result_is_valid(result) && result.outcome==AMA_HOST_YIELDED);\n"
        "AmaHandle root=host.root; uint64_t identity=host.identity;\n"
        "assert(host.active.world==NULL && host.registry.initialized && !host.disposed);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME);\n"
        "assert(ama_host_result_is_valid(result) && result.outcome==AMA_HOST_COMPLETED);\n"
        "assert(ama_handle_equal(root,host.root) && host.identity==identity);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME);\n"
        "assert(result.status.code==AMA_NO_RESUME && host.status.code==AMA_OK && !host.disposed);\n"
        "assert(host.registry.initialized && ama_handle_equal(root,host.root) && host.identity==identity);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(ama_host_result_is_valid(result) && result.outcome==AMA_HOST_YIELDED);\n"
        "assert(ama_handle_equal(root,host.root) && host.identity==identity);\n"
        "AmaStatus status=ama_project_dispose(&host);\n"
        "assert(status.code==AMA_OK && host.disposed && !host.registry.initialized && host.active.world==NULL);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.status.code==AMA_INVALID_STATE);\n"
        "status=ama_project_dispose(&host); assert(status.code==AMA_OK); return 0; }\n";
    execute_project("@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < |10\n%\nprobe !\n < |20\n", "10:1:0\n20:2:0\n10:3:0\n", true);
    host_launcher = NULL;
}

/** Accepted body failure disposes the context and keeps its original status;
 * later requests cannot resume, restart or repeat that failed native activation.
 */
static void it_keeps_admitted_execution_failure_terminal_and_never_resumes_it(void) {
    host_launcher = "#include \"project.h\"\n#include \"project.c\"\n#include <assert.h>\n"
        "int main(void) { AmaProjectHost host={0};\n"
        "AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(ama_host_result_is_valid(result) && result.outcome==AMA_HOST_FAILED && result.status.code==AMA_NATIVE_FAILURE);\n"
        "assert(host.disposed && !host.registry.initialized && host.active.world==NULL);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.status.code==AMA_INVALID_STATE);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_ENTRY); assert(result.status.code==AMA_INVALID_STATE);\n"
        "AmaStatus status=ama_project_dispose(&host); assert(status.code==AMA_NATIVE_FAILURE); return 0; }\n";
    execute_project("@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < |500\n%\nprobe !\n < |999\n", "native-entered\n", true);
    expect_diagnostic("Body failed");
    host_launcher = NULL;
}
