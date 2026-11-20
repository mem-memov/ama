/** @file
 * @brief Execute generated specialization constants and inspect isolated context tables through checked leases.
 * @see [T186 contract](../../../../docs/implementation/specialized-constants.md)
 */
#include "../execution/common.c"

/** Real native literal and copy construction must produce independent values without entering namespace bodies. */
static void it_constructs_specialized_constants_once_per_context_without_executing_namespace_bodies(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.numbers\n/test/fixture/v1.0.0/src/container\n .slot < /test/fixture/v1.0.0/src/value\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/container", "@=\nfirst\n.slot\n |17\nsecond\n.slot\n first\nempty[]\n.slot\ndefault\n.slot\n@<\ninput\n.slot\n@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n");
    host_launcher =
        "#include <assert.h>\n#include <string.h>\n#include \"project.h\"\n#include \"project.c\"\n"
        "int main(void) { AmaProjectHost hosts[2]={0}; AmaContextLease leases[2]={0}; AmaContextView views[2]={0};\n"
        "for(size_t i=0;i<2;i++) { AmaHostResult result=ama_project_execute(&hosts[i],AMA_ACCESS_ENTRY); assert(result.status.code==AMA_OK);\n"
        "assert(ama_context_enter(&hosts[i].registry,hosts[i].identity,&leases[i]).code==AMA_OK);\n"
        "assert(ama_context_inspect(&leases[i],&views[i]).code==AMA_OK); assert(views[i].program->constant_count==4);\n"
        "for(size_t j=0;j<4;j++) { const AmaConstantStorage *storage=views[i].program->constants[j].storage;\n"
        "if(j==2) { assert(storage->count==0 && !storage->items); continue; }\n"
        "assert(storage->count==1 && storage->items && storage->items[0].payload); size_t number=0;\n"
        "memcpy(&number,storage->items[0].payload,sizeof(number)); assert(number==(j==3 ? 0 : 17));\n"
        "for(size_t k=0;k<j;k++) { if(k!=2) assert(storage->items[0].payload!=views[i].program->constants[k].storage->items[0].payload); } } }\n"
        "for(size_t j=0;j<4;j++) { assert(views[0].program->constants[j].type_id==views[1].program->constants[j].type_id);\n"
        "assert(views[0].program->constants[j].storage!=views[1].program->constants[j].storage);\n"
        "if(j!=2) assert(views[0].program->constants[j].storage->items[0].payload!=views[1].program->constants[j].storage->items[0].payload); }\n"
        "for(size_t i=0;i<2;i++) { assert(ama_context_leave(&leases[i]).code==AMA_OK); assert(ama_project_dispose(&hosts[i]).code==AMA_OK); }\n"
        "puts(\"independent specialization constants\"); return EXIT_SUCCESS; }\n";
    execute_project("@!\nreplacement\n/test/fixture/v1.0.0/src/value\n |99\n/test/fixture/v1.0.0/src/factory\n .numbers\n first >> first\n second >> second\n/test/fixture/v1.0.0/src/factory\n .numbers\n second >> again\nfirst\n replacement\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n < first\nprobe !\n < second\n", "99:1:0\n17:2:0\n99:1:0\n17:2:0\nindependent specialization constants\n", true);
    host_launcher = NULL;
}

/** A failed concrete shared literal prevents both ordinary root execution and namespace-body execution. */
static void it_reports_specialized_constant_construction_failure_before_root_execution(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/factory", "@...\n.numbers\n/test/fixture/v1.0.0/src/container\n .slot < /test/fixture/v1.0.0/src/value\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/container", "@=\nfirst\n.slot\n |invalid\n@<\ninput\n.slot\n@!\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n");
    execute_project("@!\n/test/fixture/v1.0.0/src/factory\n .numbers\n first >> first\nprobe\n/test/fixture/v1.0.0/src/worker\nprobe !\n", "", false);
    expect_diagnostic("shared construction");
}

/** Every case runs explicitly; fixture teardown checks frontend/scaffold allocation balance. */
static void run(void) {
    it_constructs_specialized_constants_once_per_context_without_executing_namespace_bodies();
    it_reports_specialized_constant_construction_failure_before_root_execution();
}

/** Report success only after generated execution and all ownership assertions. */
int main(void) { run(); return 0; }
