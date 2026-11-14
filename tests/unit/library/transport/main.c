/** @file
 * @brief Generate real typed C codecs and execute a public-header-only facade consumer.
 */
#include "common.c"
#include "bridge.c"
/** Emit test-only public function prototypes alongside generated nominal C declarations. */
static void consumer_header(const LibraryBuild *build) {
    FILE *output=fopen("generated/consumer-api.h","wb"); assert(output!=NULL);
    int written=fputs("#include \"public.h\"\n",output); assert(written!=EOF);
    aliases(output,build);
    written=fputs(
        "API(ApiResult) fixture_prepare(API(Context),API(Instance),const TEXT(Borrowed)*,const NUMBER(Borrowed)*,const CONDITION(Borrowed)*,const FAILURE(Borrowed)*,const LIST(Borrowed)*);\n"
        "void fixture_other_lists(API(Context),API(Instance),const NUMBERS(Borrowed)*,const CONDITIONS(Borrowed)*,const FAILURES(Borrowed)*,NUMBERS(Value)**,CONDITIONS(Value)**,FAILURES(Value)**);\n"
        "void fixture_busy_child(API(Context),API(Instance),int);\n"
        "void fixture_yield(API(Context),API(Instance)); void fixture_check_yield(API(Context),API(Instance));\n"
        "void select_failure(size_t); size_t allocation_calls(void); size_t live_allocations(void);\n",output);
    int closed=fclose(output); assert(written!=EOF && closed==0);
}
/** Real declared profiles are decoded into final Ama fields and encoded into independent C owners. */
static void it_composes_typed_inputs_and_atomic_retained_reads_with_real_native_modules(void) {
    setup_test(); prepare_project();
    LibraryBuild foreign; component(&foreign,"foreign.component");
    LibraryBuild build; transport_model(&build);
    NativeScaffold assembly; native_scaffold_init(&assembly,&build.frontend,sources.allocator);
    bool built=project_codegen_build(&assembly,(ScaffoldOptions){.directory="generated/output",.sdk=TEST_SDK});
    if (!built) { fprintf(stderr,"%s\n",assembly.error.message); } assert(built);
    FILE *probe=tmpfile(); assert(probe!=NULL);
    bool accepted=library_codecs_write(&foreign,&assembly,probe);
    assert(!accepted && ftell(probe)==0); int closed=fclose(probe); assert(closed==0);
    bool published=native_scaffold_publish(&assembly); assert(published);
    header(&build,"generated/public.h"); bridge(&build,&assembly); consumer_header(&build);
    char errors[32768];
    bool compiled=compile("project/consumer.c",NULL,"generated/consumer.o",false,"-Igenerated",errors,sizeof(errors));
    if (!compiled) { fputs(errors,stderr); } assert(compiled);
    file("generated/wrong-schema.c","#include \"consumer-api.h\"\nvoid mismatch(API(Context) c,API(Instance) i) { TEXT(Borrowed) t={0}; (void)fixture_prepare(c,i,&t,&t,NULL,NULL,NULL); }\n");
    compiled=compile("generated/wrong-schema.c",NULL,"generated/wrong-schema.o",false,"-Igenerated",errors,sizeof(errors));
    assert(!compiled && strstr(errors,"incompatible")!=NULL);
    link_component(); execute("generated/consumer");
    native_scaffold_destroy(&assembly); library_build_destroy(&build); library_build_destroy(&foreign); destroy_test();
}
/** Register complete generated input/native/snapshot composition rather than codec test stubs. */
void run(void) { it_composes_typed_inputs_and_atomic_retained_reads_with_real_native_modules(); }
/** Return after all counted compiler models and external C resources were consumed. */
int main(void) { run(); return EXIT_SUCCESS; }
