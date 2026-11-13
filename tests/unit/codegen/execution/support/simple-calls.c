/** @file
 * @brief Exercise no-binding calls and immediate returns through two ordinary module types and independent stable instances.
 */
/** Repeated visits preserve each owned native payload; early completion returns to the correct immediate caller label. */
static void it_returns_through_nested_no_binding_calls_without_reinitializing_caller_state(void) {
    directory("generated/project");
    file("generated/project/outer", "@!\nchild\n.inner\nunused\n.worker\nchild !\n>\nunused !\n");
    file("generated/project/inner", "@!\nprobe\n.worker\nprobe !\n>\nprobe !\n");
    execute_project("@!\nfirst\n.outer\nsecond\n.outer\nfirst !\nsecond !\nfirst !\n", "7:1:0\n7:1:0\n7:2:0\n", true);
}

/** A hostile host-side fixture corrupts service metadata outside the native contract; dispatch must reject both an unknown selector and a valid selector owned by another caller type. */
static void it_rejects_unknown_and_foreign_type_return_selectors_before_dispatch(void) {
    directory("generated/project");
    file("generated/project/outer", "@!\nchild\n.inner\nchild !\n");
    file("generated/project/inner", "@!\nprobe\n.worker\nprobe !\nprobe !\n");
    file("generated/return-fault.h", "static void fault_return_link(void);\n");
    forced_header = "generated/return-fault.h";
    corrupt_return_link = true;
    for (size_t mode = 0; mode < 2; mode++) {
        char launcher[2500];
        int length = snprintf(launcher, sizeof(launcher),
            "#include <assert.h>\n#include \"project.h\"\n"
            "static AmaProjectHost *fault_host;\n"
            "static void fault_return_link(void) {\n"
            "AmaProgram *program=&fault_host->program; AmaPoolCell *worker=NULL,*inner=NULL,*outer=NULL;\n"
            "AmaStatus status=ama_program_cell(program,program->current,&worker); assert(status.code==AMA_OK);\n"
            "status=ama_program_cell(program,worker->metadata.service.return_link.caller,&inner); assert(status.code==AMA_OK);\n"
            "status=ama_program_cell(program,inner->metadata.service.return_link.caller,&outer); assert(status.code==AMA_OK);\n"
            "assert(outer->metadata.service.return_link.attached && !outer->metadata.service.return_link.host);\n"
            "worker->metadata.service.return_link.continuation=%s; }\n"
            "#include \"project.c\"\n"
            "int main(void) { AmaProjectHost host={0}; fault_host=&host;\n"
            "AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
            "assert(result.outcome==AMA_HOST_FAILED && result.status.code==AMA_INVALID_STATE);\n"
            "AmaStatus status=ama_project_dispose(&host); assert(status.code==AMA_INVALID_STATE);\n"
            "assert(!host.world.operation_active && host.disposed);\n"
            "for(size_t index=0;index<host.world.type_count;index++) assert(host.world.types[index]->pool.live_count==0);\n"
            "return EXIT_FAILURE; }\n",
            mode == 0 ? "UINT64_MAX" : "outer->metadata.service.return_link.continuation");
        assert(length > 0 && (size_t)length < sizeof(launcher));
        host_launcher = launcher;
        execute_project("@!\nouter\n.outer\nouter !\n", "7:1:0\n", false);
    }
    host_launcher = NULL;
    corrupt_return_link = false;
    forced_header = NULL;
}
