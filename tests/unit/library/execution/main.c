/** @file
 * @brief Execute actual exported Ama label bodies from an independent C consumer.
 */
#include "../transport/common.c"
#include "native.c"
#include "bridge.c"
/** Every selected mode is explicit, including a read-only alias that cannot use another export's control. */
static const char exports[]=
    "export | counter | .counter | entry,resume,read\ninput | counter | message | text\noutput | counter | flag | enabled\n"
    "export | observer | .main | read\n"
    "output | observer | message result | text\noutput | observer | number result | number\n"
    "output | observer | enabled result | enabled\noutput | observer | problem result | failure\n"
    "output | observer | messages result | texts\noutput | observer | message copy | text\n";
/** A yielded Ama root, a native method root and a read-only export share one checked component. */
static void it_executes_exported_entries_resumes_and_isolated_native_roots(void) {
    setup_test(); prepare_project(); counter_sources();
    LibraryBuild foreign; component(&foreign,"foreign.component"); library_build_destroy(&foreign);
    LibraryBuild build;
    transport_model_body(&build,
        "off\n..std.condition\n |0\nmessages result[]\n messages[]\n%\nenabled result\n off\n",exports);
    NativeScaffold assembly; native_scaffold_init(&assembly,&build.frontend,sources.allocator);
    bool built=project_codegen_build(&assembly,(ScaffoldOptions){.directory="generated/output",.sdk=TEST_SDK});
    if (!built) { fprintf(stderr,"%s\n",assembly.error.message); } assert(built);
    counter_body(&assembly);
    bool published=native_scaffold_publish(&assembly); assert(published);
    header(&build,"generated/public.h"); bridge(&build,&assembly); consumer_header(&build);
    char errors[32768];
    bool compiled=compile("project/consumer.c",NULL,"generated/consumer.o",false,"-Igenerated",errors,sizeof(errors));
    if (!compiled) { fputs(errors,stderr); } assert(compiled);
    file("generated/wrong-mode.c","#include \"consumer-api.h\"\nvoid mismatch(API(Context) c,API(Instance) i) { (void)OBSERVER(entry)(c,i); }\n");
    compiled=compile("generated/wrong-mode.c",NULL,"generated/wrong-mode.o",false,"-Igenerated",errors,sizeof(errors));
    assert(!compiled && strstr(errors,"implicit declaration")!=NULL);
    link_component(); execute("generated/consumer");
    link_component_source_configuration("project/consumer.c",true); execute("generated/consumer");
    native_scaffold_destroy(&assembly); library_build_destroy(&build); destroy_test();
}
/** Empty interfaces exercise every selected mode without typed argument/output arrays. */
static void it_generates_entry_resume_and_read_for_a_zero_schema_interface(void) {
    setup_test(); prepare_project(); file("generated/project/main","@!\n%\n");
    const SourceText *source=configuration("export | run | .main | entry,resume,read\n");
    LibraryBuild build; library_build_init(&build,sources.allocator);
    bool valid=library_configuration_parse(&build,source); assert(valid);
    ModuleRoot root={.path="generated/project",.kind=MODULE_ROOT_PROJECT};
    valid=library_build_resolve(&build,&root,1); assert(valid);
    NativeScaffold assembly; native_scaffold_init(&assembly,&build.frontend,sources.allocator);
    bool built=project_codegen_build(&assembly,(ScaffoldOptions){.directory="generated/output",.sdk=TEST_SDK}); assert(built);
    bool published=native_scaffold_publish(&assembly); assert(published);
    header(&build,"generated/public.h"); bridge(&build,&assembly);
    FILE *output=fopen("generated/consumer-api.h","wb"); assert(output!=NULL);
    int written=fprintf(output,"#include \"public.h\"\n#define API(name) %s_ ## name\n#define EXPORTED(name) %s_ ## name\nsize_t live_allocations(void);\n",
        build.identity->internal_name,build.exports->identity->internal_name);
    int closed=fclose(output); assert(written>0 && closed==0);
    char errors[32768];
    bool compiled=compile("project/empty.c",NULL,"generated/consumer.o",false,"-Igenerated",errors,sizeof(errors));
    if (!compiled) { fputs(errors,stderr); } assert(compiled);
    link_component_source("project/empty.c"); execute("generated/consumer");
    link_component_source_configuration("project/empty.c",true); execute("generated/consumer");
    native_scaffold_destroy(&assembly); library_build_destroy(&build); destroy_test();
}
/** Register real public entry/resume composition, native reentry and every admitted allocation failure. */
void run(void) {
    it_executes_exported_entries_resumes_and_isolated_native_roots();
    it_generates_entry_resume_and_read_for_a_zero_schema_interface();
}
/** Return after all generated host calls and counted ownership assertions complete. */
int main(void) { run(); return EXIT_SUCCESS; }
