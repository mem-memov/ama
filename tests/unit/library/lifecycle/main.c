/** @file
 * @brief Compile real project descriptors and checked public lifecycle thunks with a C host.
 */
#include "../header/common.c"
#include "codegen/internal.h"
#include "codegen/codegen.h"
#include "library/transport/codec.h"
/** Emit the private bridge using exact scaffold descriptor ordinals, never source-name guesses. */
static void private_bridge(const LibraryBuild *build, NativeScaffold *assembly) {
    uint64_t types[2];
    size_t count = 0;
    for (const LibraryExport *e = build->exports; e != NULL; e = e->next) {
        assert(count < sizeof(types)/sizeof(*types));
        const ScaffoldType *type = native_scaffold_type(assembly,e->type);
        assert(type != NULL);
        types[count++] = codegen_type_id(assembly,type);
    }
    FILE *out = fopen("generated/component.c","wb");
    assert(out != NULL);
    int written = fputs("#include \"public.h\"\n#include <stdlib.h>\n"
        "static int fail_allocation;\nvoid fail_next_allocation(void) { fail_allocation=1; }\n"
        "static void *lifecycle_malloc(size_t size) { if(fail_allocation) { fail_allocation=0; return NULL; } return malloc(size); }\n"
        "#define malloc lifecycle_malloc\n#include \"output/project.c\"\n",out);
    assert(written != EOF);
    bool good = library_abi_write(build,out);
    assert(good);
    good = library_lifecycle_write(build,types,count,out);
    assert(good);
    good = library_snapshots_write(build,out); assert(good);
    good = library_codecs_write(build,assembly,out);
    int closed = fclose(out);
    assert(good && closed == 0);
}
/** A separate consumer sees only public declarations and exercises real generated ownership. */
static void consumer(const LibraryBuild *build) {
    FILE *out = fopen("generated/consumer.c","wb");
    assert(out != NULL);
    int written = fprintf(out,
        "#include \"public.h\"\n#include <assert.h>\n#include <string.h>\n"
        "#define API(name) %s_ ## name\n#define CREATE %s_create\n#define READ %s_read\n#define FAILURE(name) %s_ ## name\n"
        "extern void fail_next_allocation(void);\n"
        "static void ok(API(ApiResult) result) { assert(result.code==0); assert(API(result_release)(&result)==0); }\n"
        "int main(void) {\n"
        " API(Context) a={0}, b={0}; API(Instance) x={0}, y={0};\n"
        " fail_next_allocation(); API(ApiResult) failed=API(context_create)(&a);\n"
        " assert(failed.code==5 && failed.diagnostic.code!=0 && failed.diagnostic.length!=0 && a.opaque[0]==0 && a.opaque[1]==0);\n"
        " assert(API(result_release)(&failed)==0);\n"
        " ok(API(context_create)(&a)); ok(API(context_create)(&b));\n"
        " ok(CREATE(a,&x)); ok(CREATE(b,&y));\n"
        " FAILURE(Value) *snapshot=NULL, *second=NULL; FAILURE(Borrowed) view={0};\n"
        " ok(READ(a,x,&snapshot)); ok(READ(a,x,&second)); assert(snapshot!=second);\n"
        " ok(FAILURE(view)(snapshot,&view)); assert(view.failed==0 && view.description.length==0);\n"
        " API(Instance) old=x; API(Context) stale=a;\n"
        " API(ApiResult) result=API(instance_dispose)(b,&x); assert(result.code==1);\n"
        " assert(memcmp(&x,&old,sizeof(x))==0); ok((API(ApiResult)){0});\n"
        " assert(API(result_release)(&result)==0);\n"
        " API(Diagnostic) diagnostic={0}; ok(API(diagnostic)(a,&diagnostic));\n"
        " assert(diagnostic.code==0 && diagnostic.length==0); assert(API(diagnostic_release)(&diagnostic)==0);\n"
        " ok(API(instance_dispose)(a,&x)); assert(x.opaque[0]==0 && x.opaque[1]==0 && x.opaque[2]==0);\n"
        " result=API(instance_dispose)(a,&old); assert(result.code==1); assert(API(result_release)(&result)==0);\n"
        " ok(API(context_dispose)(&a)); assert(a.opaque[0]==0 && a.opaque[1]==0);\n"
        " ok(FAILURE(view)(snapshot,&view)); assert(view.failed==0 && view.description.length==0);\n"
        " ok(FAILURE(release)(&snapshot)); ok(FAILURE(release)(&second));\n"
        " result=CREATE(stale,&x); assert(result.code==1); assert(API(result_release)(&result)==0);\n"
        " ok(API(context_create)(&a)); assert(a.opaque[1]!=stale.opaque[1]);\n"
        " ok(API(context_dispose)(&a)); ok(API(context_dispose)(&b));\n"
        " ok(API(context_create)(&a)); fail_next_allocation(); failed=CREATE(a,&x);\n"
        " assert(failed.code==5 && x.opaque[0]==0 && x.opaque[1]==0 && x.opaque[2]==0);\n"
        " diagnostic=(API(Diagnostic)){0}; ok(API(diagnostic)(a,&diagnostic));\n"
        " assert(diagnostic.code!=0 && diagnostic.length==failed.diagnostic.length);\n"
        " API(ApiResult) cleanup=API(context_dispose)(&a); assert(cleanup.code==5);\n"
        " assert(a.opaque[0]==0 && a.opaque[1]==0 && diagnostic.message!=NULL);\n"
        " assert(memcmp(diagnostic.message,failed.diagnostic.message,diagnostic.length)==0);\n"
        " assert(API(result_release)(&failed)==0 && API(result_release)(&cleanup)==0 && API(diagnostic_release)(&diagnostic)==0);\n"
        " assert(API(diagnostic_release)(NULL)==1); assert(API(result_release)(NULL)==1); return 0;\n}\n",
        build->identity->internal_name,build->exports->identity->internal_name,
        build->exports->identity->internal_name,build->schemas->next->next->next->identity->internal_name);
    int closed = fclose(out);
    assert(written > 0 && closed == 0);
}
/** Compile the private assembly with its two explicit SDK/declaration search roots. */
static void link_component(void) {
    pid_t child = fork(); assert(child >= 0);
    if (child == 0) {
        execlp(TEST_C_COMPILER,TEST_C_COMPILER,"-std=c11","-O2","-Wall","-Wextra","-Werror","-pedantic-errors",
            "-I" TEST_SDK,"-Igenerated/output","generated/consumer.c","generated/component.c","-o","generated/consumer",(char *)NULL);
        _exit(127);
    }
    int status; pid_t waited;
    do { waited = waitpid(child,&status,0); } while (waited < 0 && errno == EINTR);
    assert(waited == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** The public API links to an actual single-TU Ama assembly; no test stub implements lifecycle. */
static void it_links_public_lifecycle_to_real_generated_contexts(void) {
    setup_test(); prepare_project();
    LibraryBuild build; component(&build,"lifecycle.component");
    NativeScaffold assembly;
    native_scaffold_init(&assembly,&build.frontend,sources.allocator);
    bool built = project_codegen_build(&assembly,(ScaffoldOptions){.directory="generated/output",.sdk=TEST_SDK});
    if (!built) { fprintf(stderr,"%s\n",assembly.error.message); }
    assert(built);
    bool published = native_scaffold_publish(&assembly);
    assert(published);
    header(&build,"generated/public.h");
    private_bridge(&build,&assembly); consumer(&build);
    char errors[32768];
    bool compiled = compile("generated/consumer.c",NULL,"generated/consumer.o",false,NULL,errors,sizeof(errors));
    if (!compiled) { fputs(errors,stderr); }
    assert(compiled);
    link_component();
    execute("generated/consumer");
    native_scaffold_destroy(&assembly); library_build_destroy(&build); destroy_test();
}
/** Reject incomplete ordinal maps before emitting any public-looking definition. */
static void it_rejects_incomplete_lifecycle_type_maps(void) {
    setup_test(); prepare_project();
    LibraryBuild build; component(&build,"lifecycle.component");
    FILE *out = tmpfile(); assert(out != NULL);
    uint64_t ids[] = {1,0};
    bool written = library_lifecycle_write(&build,ids,2,out);
    assert(!written && ftell(out)==0);
    written = library_lifecycle_write(&build,ids,1,out);
    assert(!written && ftell(out)==0);
    int closed = fclose(out); assert(closed==0);
    library_build_destroy(&build); destroy_test();
}
/** Register composed public linkage and pre-emission mapping rejection. */
void run(void) {
    it_links_public_lifecycle_to_real_generated_contexts();
    it_rejects_incomplete_lifecycle_type_maps();
}
/** Return after counted compiler owners and independently generated C resources were released. */
int main(void) { run(); return EXIT_SUCCESS; }
