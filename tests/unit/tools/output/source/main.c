/** @file
 * @brief Verify original flat/context diagnostic notes in JSON without allocation or invalid partial output.
 * @see [T192 contract](../../../../../docs/implementation/static-editor-bindings.md)
 */
#include <sys/wait.h>
#include <unistd.h>
#include "../../../source/common.c"
#include "tools/output/source/source.h"

/** An independent decoder verifies explicit note order and exact original
 * Unicode paths; every SourceAllocator call is forbidden after the cause. */
static void it_preserves_flat_and_context_diagnostic_attribution_without_failure_allocations(void) {
    setup_test();
    const SourceText *source;
    const char *text = "first\nsecond\n";
    assert(source_manager_add(&sources,"данные-имя",(const unsigned char *)text,strlen(text),&source) == SOURCE_OK);
    DiagnosticRelated original = {{.span={source,0,5}},"original related",".definition"};
    Diagnostic error = {.code=DIAGNOSTIC_TYPE_MISMATCH,.message="Static mismatch",.primary={.span={source,0,5}},
        .related=&original,.related_count=1};
    DiagnosticContext caller = {0}, root = {0};
    assert(diagnostic_add_context(&error,&caller,(DiagnosticRelated){{.span={source,6,12}},"caller",".caller"}));
    assert(diagnostic_add_context(&error,&root,(DiagnosticRelated){{.span={source,6,12}},"root",".root"}));
    FILE *output = fopen("generated/diagnostic.json","wb");
    assert(output);
    ToolJsonWriter writer;
    tool_json_init(&writer,output);
    size_t before = allocation_count;
    failure_at = before + 1;
    assert(tool_source_diagnostic(&writer,&error,false) && writer.status == TOOL_JSON_OK);
    assert(allocation_count == before && fclose(output) == 0);
    failure_at = 0;
    pid_t process = fork();
    assert(process >= 0);
    if (!process) { execlp("python3","python3","verify.py","generated/diagnostic.json",(char *)NULL); _exit(127); }
    int status;
    assert(waitpid(process,&status,0) == process && WIFEXITED(status) && WEXITSTATUS(status) == 0);
    destroy_test();
}

/** A cycle or absent primary error is rejected before JSON begins; the first
 * writer failure remains sticky rather than being replaced by validation. */
static void it_refuses_cyclic_or_invalid_diagnostics_before_any_output(void) {
    DiagnosticContext cycle = {.related={.role="cycle"}};
    cycle.next = &cycle;
    Diagnostic error = {.code=DIAGNOSTIC_TYPE_MISMATCH,.message="Invalid context",.contexts=&cycle};
    FILE *output = tmpfile();
    assert(output);
    ToolJsonWriter writer;
    tool_json_init(&writer,output);
    assert(!tool_source_diagnostic(&writer,&error,false));
    assert(writer.status == TOOL_JSON_INVALID_VALUE && ftell(output) == 0);
    writer.status = TOOL_JSON_IO_ERROR;
    assert(!tool_source_diagnostic(&writer,&error,false) && writer.status == TOOL_JSON_IO_ERROR);
    tool_json_init(&writer,output);
    assert(!tool_source_diagnostic(&writer,NULL,false));
    assert(writer.status == TOOL_JSON_INVALID_VALUE && ftell(output) == 0);
    assert(fclose(output) == 0);
}

/** Run real source attribution and malformed-record partitions explicitly. */
static void run(void) {
    it_preserves_flat_and_context_diagnostic_attribution_without_failure_allocations();
    it_refuses_cyclic_or_invalid_diagnostics_before_any_output();
}

/** Return success only after borrowed source and counted allocator teardown. */
int main(void) { run(); return 0; }
