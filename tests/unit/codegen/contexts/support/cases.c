/** @file
 * @brief Execute isolated generated hosts concurrently and inspect disjoint pools/constants only through live context leases.
 */
/** Separate contexts may yield simultaneously, retain separate shared storage and reject foreign checked references even when local handle triples coincide. */
static void it_interleaves_independent_generated_contexts_with_private_constants_and_pools(void) {
    host_launcher = "#include <assert.h>\n#include \"project.c\"\n"
        "static void require(bool value) { assert(value); (void)value; }\n"
        "int main(void) { AmaProjectHost first={0},second={0};\n"
        "AmaHostResult result=ama_project_execute(&first,AMA_ACCESS_RESUME);\n"
        "require(result.status.code==AMA_NO_RESUME && !first.initialized && !first.disposed);\n"
        "result=ama_project_execute(&first,AMA_ACCESS_ENTRY); require(result.outcome==AMA_HOST_YIELDED);\n"
        "result=ama_project_execute(&second,AMA_ACCESS_ENTRY); require(result.outcome==AMA_HOST_YIELDED);\n"
        "require(first.identity!=second.identity && ama_handle_equal(first.root,second.root));\n"
        "AmaContextLease left={0},right={0}; AmaContextView a={0},b={0};\n"
        "require(ama_context_enter(&first.registry,first.identity,&left).code==AMA_OK);\n"
        "require(ama_context_enter(&second.registry,second.identity,&right).code==AMA_OK);\n"
        "require(ama_context_inspect(&left,&a).code==AMA_OK && ama_context_inspect(&right,&b).code==AMA_OK);\n"
        "require(a.world!=b.world && a.program!=b.program && a.world->types[0]!=b.world->types[0]);\n"
        "require(a.program->constants[0].storage!=b.program->constants[0].storage);\n"
        "require(a.program->constants[0].storage->items!=b.program->constants[0].storage->items);\n"
        "AmaObjectType *type=NULL; AmaPoolCell *cell=NULL;\n"
        "require(ama_context_resolve(&left,(AmaContextReference){second.identity,second.root},AMA_ACCESS_READ,&type,&cell).code==AMA_TYPE_MISMATCH && type==NULL && cell==NULL);\n"
        "require(ama_project_dispose(&first).code==AMA_BUSY && !first.disposed && first.status.code==AMA_OK);\n"
        "result=ama_project_execute(&first,AMA_ACCESS_RESUME); require(result.status.code==AMA_BUSY && !first.disposed);\n"
        "require(ama_context_leave(&right).code==AMA_OK && ama_context_leave(&left).code==AMA_OK);\n"
        "result=ama_project_execute(&first,AMA_ACCESS_RESUME); require(result.outcome==AMA_HOST_COMPLETED);\n"
        "require(ama_project_dispose(&first).code==AMA_OK && !first.registry.initialized);\n"
        "result=ama_project_execute(&second,AMA_ACCESS_RESUME); require(result.outcome==AMA_HOST_COMPLETED);\n"
        "require(ama_project_dispose(&second).code==AMA_OK && !second.registry.initialized); return 0; }\n";
    execute_project("@=\nshared\n.value\n |23\n@!\nprobe\n.worker\nprobe !\n%\nprobe !\n", "7:1:0\n7:1:0\n7:2:0\n7:2:0\n", true);
    host_launcher = NULL;
}

/** A host barrier proves two actual execution intervals overlap; same-host reentry is refused while independent contexts complete on separate C threads. */
static void it_executes_two_generated_contexts_in_parallel_and_refuses_same_host_reentry(void) {
    directory("generated/project");
    native_fixture("gate", "@C\n");
    file("generated/context-gate.h", "static void test_execution_gate(void);\n");
    forced_header = "generated/context-gate.h";
    native_execution_gate = true;
    host_launcher = "#include <assert.h>\n#include <threads.h>\n#include \"project.c\"\n"
        "static void require(bool value) { assert(value); (void)value; }\n"
        "static _Atomic unsigned arrivals; static _Thread_local AmaProjectHost *thread_host;\n"
        "static void test_execution_gate(void) {\n"
        "AmaHostResult refused=ama_project_execute(thread_host,AMA_ACCESS_ENTRY); require(refused.status.code==AMA_BUSY);\n"
        "require(ama_project_dispose(thread_host).code==AMA_BUSY);\n"
        "atomic_fetch_add_explicit(&arrivals,1,memory_order_release);\n"
        "while(atomic_load_explicit(&arrivals,memory_order_acquire)!=2) thrd_yield(); }\n"
        "static int execute_thread(void *argument) { AmaProjectHost *host=argument; thread_host=host;\n"
        "AmaHostResult result=ama_project_execute(host,AMA_ACCESS_ENTRY);\n"
        "require(result.outcome==AMA_HOST_COMPLETED && result.status.code==AMA_OK);\n"
        "require(host->active.world==NULL && ama_project_dispose(host).code==AMA_OK); return 0; }\n"
        "int main(void) { AmaProjectHost hosts[2]={0}; thrd_t threads[2];\n"
        "for(size_t index=0;index<2;index++) require(thrd_create(&threads[index],execute_thread,&hosts[index])==thrd_success);\n"
        "for(size_t index=0;index<2;index++) { int result=-1; require(thrd_join(threads[index],&result)==thrd_success && result==0); }\n"
        "require(atomic_load(&arrivals)==2 && hosts[0].identity!=hosts[1].identity);\n"
        "require(!hosts[0].registry.initialized && !hosts[1].registry.initialized); return 0; }\n";
    execute_project("@=\nshared\n.value\n |23\n@!\ngate\n.gate\ngate !\n", "", true);
    host_launcher = NULL;
    native_execution_gate = false;
    forced_header = NULL;
}

/** The same numeric Busy code can be a native destructor's technical failure; accepted destruction consumes the host and must not be retried as a refusal. */
static void it_distinguishes_busy_native_destruction_from_refused_host_disposal(void) {
    fail_native_destroy = true;
    busy_native_destroy = true;
    host_launcher = "#include <assert.h>\n#include \"project.c\"\n"
        "static void require(bool value) { assert(value); (void)value; }\n"
        "int main(void) { AmaProjectHost host={0}; AmaHostResult result=ama_project_execute(&host,AMA_ACCESS_ENTRY);\n"
        "require(result.outcome==AMA_HOST_COMPLETED && result.status.code==AMA_OK);\n"
        "AmaStatus status=ama_project_dispose(&host);\n"
        "require(status.code==AMA_BUSY && host.disposed && !host.registry.initialized);\n"
        "AmaStatus repeated=ama_project_dispose(&host);\n"
        "require(repeated.code==status.code && repeated.message==status.message);\n"
        "result=ama_project_execute(&host,AMA_ACCESS_ENTRY); require(result.status.code==AMA_INVALID_STATE);\n"
        "return EXIT_FAILURE; }\n";
    execute_project("@!\nprobe\n.worker\nprobe !\n", "7:1:0\n", false);
    expect_diagnostic("cleanup");
    host_launcher = NULL;
    fail_native_destroy = false;
    busy_native_destroy = false;
}
