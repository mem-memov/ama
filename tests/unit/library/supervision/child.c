/** @file
 * @brief Package a multi-yield child with an explicit scalar-only native C dependency.
 */
/** The observer object is an explicit foreign C build input, not hidden Ama state. */
static void build_probe(void) {
    char *arguments[20]={TEST_C_COMPILER,"-std=c11","-O1","-g","-Wall","-Wextra","-Werror","-pedantic-errors","-fPIC"};
    size_t count=9;
#ifdef TEST_SANITIZERS
    arguments[count++]="-fsanitize=address,undefined"; arguments[count++]="-fno-omit-frame-pointer";
#endif
    arguments[count++]="-c"; arguments[count++]="probe.c"; arguments[count++]="-o";
    arguments[count++]="generated/probe.o"; arguments[count]=NULL;
    bool compiled=library_package_process(arguments,stderr); assert(compiled);
}
/** Resolve exact native authoring metadata for each independently named probe/worker type. */
static void child_authoring(NativeScaffold *assembly,const char *module,const char *tag,const char *path) {
    const ScaffoldType *type=native_scaffold_type(assembly,semantic_find_type(&assembly->frontend->semantic,module)); assert(type!=NULL);
    authoring_aliases(type,tag,path);
}
/** Build the complete child once, retaining its distinct context domain and C transport ABI. */
static void build_supervision_child(LibraryBuild *build,NativeScaffold *assembly) {
    build_probe();
    nested_model(build,"supervision.child","child/project",
        "schema | text | text | //text\nschema | condition | condition | //std/condition\n"
        "export | run | /test/fixture/v1.0.0/src/main | entry,resume,read\ninput | run | request | text\n"
        "output | run | response | text\noutput | run | previous | condition\n"
        "link-file | generated/probe.o\ndependency-file | probe.h\n"
#ifdef TEST_SANITIZERS
        "c-flag | -fsanitize=address,undefined\nc-flag | -fno-omit-frame-pointer\nlink-flag | -fsanitize=address,undefined\n"
#endif
        );
    directory("generated/child-output"); native_scaffold_init(assembly,&build->frontend,sources.allocator);
    bool built=project_codegen_build(assembly,(ScaffoldOptions){.directory="generated/child-output",.sdk=TEST_SDK});
    if(!built) { fprintf(stderr,"%s\n",assembly->error.message); } assert(built);
    child_authoring(assembly,"/test/fixture/v1.0.0/src/probe","AMA_PROBE","generated/child-output/probe-api.h");
    child_authoring(assembly,"/test/fixture/v1.0.0/src/worker","AMA_WORKER","generated/child-output/worker-api.h");
    const ScaffoldType *text=native_scaffold_type(assembly,semantic_find_type(&build->frontend.semantic,"//text")); assert(text!=NULL);
    FILE *aliases=fopen("generated/child-output/worker-api.h","ab"); assert(aliases!=NULL);
    int written=fprintf(aliases,"#define AMA_WORKER_TEXT(n) %s_ ## n\n",text->native_prefix->internal_name);
    int closed=fclose(aliases); assert(written>0 && closed==0);
    bool published=native_scaffold_publish(assembly); assert(published);
    nested_header(build,"generated/child-output/public.h");
    public_aliases(build,"CHILD","child-output/public.h","generated/child-api.h");
    FILE *output=fopen("generated/child-output/component.c","wb"); assert(output!=NULL);
    bool emitted=library_assembly_write(build,assembly,"public.h","project.c",output);
    closed=fclose(output); assert(emitted && closed==0);
    LibraryPackageOptions options={.assembly=assembly,.compiler=TEST_C_COMPILER,.archiver="ar",.object_copy="objcopy",
        .source="generated/child-output/component.c",.sdk=TEST_SDK,.includes="generated/child-output",
        .raw_object="generated/child-output/raw.o",.isolated_object="generated/child-output/isolated.o",
        .symbols="generated/child-output/symbols.txt",.archive="generated/child-output/library.a",.shared="generated/child-output/library.so"};
    built=library_package_build(build,&options,stderr); assert(built);
}
