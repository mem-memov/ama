/** @file
 * @brief Compile a real suspended traversal and inspect its declared state without executing it.
 */
#include "../execution/common.c"

/** The consumer is authored source; generated assembly remains an opaque compilation input. */
static void it_inspects_yielded_traversals_with_original_names(void) {
    FILE *input = fopen("consumer.c", "rb");
    assert(input != NULL);
    char consumer[16384];
    size_t length = fread(consumer,1,sizeof(consumer)-1,input);
    assert(!ferror(input) && length < sizeof(consumer)-1);
    assert(fclose(input) == 0);
    consumer[length] = 0;
    host_launcher = consumer;
    execute_project("@!\nсписок[]\n/test/fixture/v1.0.0/src/value\n |4\n |5\nсписок[элемент&]\n{\n %\n}\n",
        "debugger accepted\n", true);
    host_launcher = NULL;
}
/** An accepted native failure disposes the generated host rather than exposing historical objects. */
static void it_rejects_inspection_after_generated_host_failure_cleanup(void) {
    host_launcher = "#include <stdlib.h>\n#include \"project.h\"\n#include \"project.c\"\n"
        "int main(void) { AmaProjectHost host={0}; AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "if(result.outcome!=AMA_HOST_FAILED || !host.disposed) abort();\n"
        "AmaDebugSession session={0}; if(ama_debug_begin(&host.registry,host.identity,&ama_project_debug_catalog,NULL,0,&session).code==AMA_OK) abort();\n"
        "if(ama_project_dispose(&host).code!=result.status.code) abort(); return 0; }\n";
    execute_project("@!\nworker\n/test/fixture/v1.0.0/src/worker\nworker !\n < |500\n", "native-entered\n", true);
    host_launcher = NULL;
    expect_diagnostic("Body failed");
}
/** Run the complete source-to-compiled-inspection contracts. */
void run(void) {
    it_inspects_yielded_traversals_with_original_names();
    it_rejects_inspection_after_generated_host_failure_cleanup();
}
/** Exit only after generated consumer assertions and fixture cleanup succeed. */
int main(void) { run(); return EXIT_SUCCESS; }
