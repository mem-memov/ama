/** @file
 * @brief Own real native resources and fail a later clone after an earlier copied payload completed.
 */
/** Each successful constructor owns exactly one allocation. A failing clone
 * cleans its own partial allocation; the runtime must destroy only prior credits.
 */
static const char *resource_hook(const ScaffoldType *type, AbiOperation operation) {
    if (strcmp(type->plan->type->module->source->logical_path, ".payload") != 0) { return NULL; }
    if (operation == ABI_DEFAULT) {
        return " { *destination=(AMA_NATIVE(State)){.address=destination}; destination->resource=malloc(16); if(destination->resource==NULL) return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); memset(destination->resource,0xA5,16); return ama_status_success(); }\n";
    }
    if (operation == ABI_LITERAL) {
        return " { (void)destination; (void)literal; return ama_status_make(AMA_INVALID_LITERAL,NULL,0); }\n";
    }
    if (operation == ABI_CLONE) {
        return " { assert(source->address==source && source->resource!=NULL && source->resource[15]==0xA5); "
            "printf(\"clone=%zu\\n\",source->calls); *destination=*source; destination->address=destination; "
            "destination->resource=malloc(16); if(destination->resource==NULL) return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); "
            "memset(destination->resource,0xA5,16); if(source->calls==2) { free(destination->resource); destination->resource=NULL; "
            "puts(\"partial=2\"); return ama_status_make(AMA_NATIVE_FAILURE,\"Native clone failed\",sizeof(\"Native clone failed\")-1); } "
            "return ama_status_success(); }\n";
    }
    if (operation == ABI_DESTROY) {
        return " { assert(state->address==state && state->resource!=NULL && state->resource[15]==0xA5); "
            "size_t calls=state->calls; free(state->resource); state->resource=NULL; "
            "return printf(\"drop=%zu\\n\",calls)<0 ? ama_status_make(AMA_IO_FAILURE,NULL,0) : ama_status_success(); }\n";
    }
    return NULL;
}

/** A failed later clone unwinds the prepared first resource, then terminal
 * context cleanup sees unchanged old destination and original source counters.
 * Failed contexts cannot turn the saved source position into a resumable failure.
 */
static void it_unwinds_native_clone_prefixes_preserves_old_values_and_disposes_failed_contexts(void) {
    directory("generated/project");
    native_fixture("payload", "@<\ninput\n.value\n |7\n@>\noutput\n.value\n input\nother\n.value\n |99\n@C\n");
    file("generated/project/payload.h", "struct AMA_NATIVE(State) { size_t number; size_t calls; const void *address; unsigned char *resource; };\n");
    file("generated/project/task", "@!\nfirst\n.payload\nsecond\n.payload\nfirst !\nsecond !\nsecond !\n%\nfirst !\n");
    native_hook_override = resource_hook;
    host_launcher = "#include \"project.h\"\n#include \"project.c\"\n#include <assert.h>\n"
        "int main(void) { AmaProjectHost host={0}; AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "assert(ama_host_result_is_valid(result) && result.outcome==AMA_HOST_FAILED && result.status.code==AMA_NATIVE_FAILURE);\n"
        "assert(host.disposed && !host.registry.initialized && host.active.world==NULL);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_RESUME); assert(result.status.code==AMA_INVALID_STATE);\n"
        "AmaStatus status=ama_project_dispose(&host); assert(status.code==AMA_NATIVE_FAILURE); return 0; }\n";
    execute_project("@!\nsource\n.task\ndestination\n.task\nsource !\ndestination\n % source\nsource %\n",
        "7:1:0\n7:1:0\n7:2:0\nclone=1\nclone=2\npartial=2\ndrop=1\ndrop=0\ndrop=0\ndrop=2\ndrop=1\n", true);
    expect_diagnostic("Native clone failed");
    host_launcher = NULL;
    native_hook_override = NULL;
}
