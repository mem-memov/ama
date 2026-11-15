/** @file
 * @brief Compose one parent native adapter against an independently packaged child archive.
 */
/** Test hooks expose only C controls/metrics; no retained pointer points into an Ama field. */
static void parent_probe(FILE *output) {
    int written=fputs(
        "#include \"parent-api.h\"\n#include \"child-api.h\"\n#include <stdlib.h>\n#include <assert.h>\n"
        "static size_t allocation_calls,live_allocations,fail_at;\n"
        "void select_failure(size_t ordinal) { allocation_calls=0; fail_at=ordinal; }\n"
        "size_t allocations(void) { return allocation_calls; }\nsize_t live(void) { return live_allocations; }\n"
        "static void *parent_malloc(size_t size) { allocation_calls++; if(allocation_calls==fail_at) return NULL; void *p=malloc(size); if(p!=NULL) live_allocations++; return p; }\n"
        "static void parent_free(void *p) { if(p!=NULL) { assert(live_allocations!=0); live_allocations--; free(p); } }\n"
        "extern CHILD(ApiResult) fixture_context_create(CHILD(Context) *);\n"
        "extern CHILD(ApiResult) fixture_instance_create(CHILD(Context),CHILD(Instance) *);\n"
        "extern void fixture_created(CHILD(Context)); extern void fixture_disposed(CHILD(Context));\n"
        "extern void fixture_before_child(CHILD(Context),CHILD(Instance));\n"
        "#define AMA_EMBED_CONTEXT_CREATE fixture_context_create\n#define AMA_EMBED_CREATE fixture_instance_create\n"
        "#define AMA_EMBED_CREATED fixture_created\n#define AMA_EMBED_DISPOSED fixture_disposed\n"
        "#define AMA_EMBED_BEFORE_CHILD fixture_before_child\n"
        "#define malloc parent_malloc\n#define free parent_free\n",output); assert(written!=EOF);
}
/** Native parent construction owns no implicit child body and no native clone capability. */
static void build_parent(LibraryBuild *build, NativeScaffold *assembly) {
    nested_model(build,"nested.parent","project",
        "schema | text | text | ..text\nschema | condition | condition | ..std.condition\n"
        "export | run | .adapter | entry,read\ninput | run | message | text\n"
        "output | run | response | text\noutput | run | previous | condition\noutput | run | waiting | condition\n"
        "link-file | generated/child-output/library.a\n");
    directory("generated/parent-output");
    native_scaffold_init(assembly,&build->frontend,sources.allocator);
    bool built=project_codegen_build(assembly,(ScaffoldOptions){.directory="generated/parent-output",.sdk=TEST_SDK});
    if(!built) { fprintf(stderr,"%s\n",assembly->error.message); } assert(built);
    const ScaffoldType *adapter=native_scaffold_type(assembly,semantic_find_type(&build->frontend.semantic,".adapter"));
    const ScaffoldType *text=native_scaffold_type(assembly,semantic_find_type(&build->frontend.semantic,"..text"));
    assert(adapter!=NULL && text!=NULL);
    authoring_aliases(adapter,"AMA_EMBED","generated/adapter-api.h");
    FILE *aliases=fopen("generated/adapter-api.h","ab"); assert(aliases!=NULL);
    int written=fprintf(aliases,"#include \"child-api.h\"\n#define AMA_EMBED_TEXT(n) %s_ ## n\n",text->native_prefix->internal_name);
    int closed=fclose(aliases); assert(written>0 && closed==0);
    bool published=native_scaffold_publish(assembly); assert(published);
    nested_header(build,"generated/parent-output/public.h");
    public_aliases(build,"PARENT","parent-output/public.h","generated/parent-api.h");
    FILE *output=fopen("generated/parent.c","wb"); assert(output!=NULL); parent_probe(output);
    bool emitted=library_assembly_write(build,assembly,"parent-output/public.h","parent-output/project.c",output);
    closed=fclose(output); assert(emitted && closed==0);
}
/** Compile the consumer without any SDK include path, proving public-boundary independence. */
static void compile_consumer(void) {
    char *arguments[22]={TEST_C_COMPILER,"-std=c11","-O1","-g","-Wall","-Wextra","-Werror","-pedantic-errors"};
    size_t count=8;
#ifdef TEST_SANITIZERS
    arguments[count++]="-fsanitize=address,undefined"; arguments[count++]="-fno-omit-frame-pointer";
    arguments[count++]="-fno-pie";
#endif
    arguments[count++]="-Igenerated"; arguments[count++]="-c";
    arguments[count++]="consumer/main.c"; arguments[count++]="-o";
    arguments[count++]="generated/consumer.o"; arguments[count]=NULL;
    bool compiled=library_package_process(arguments,stderr); assert(compiled);
}
/** Link separate public-only host C and parent/child compiled code with matching sanitizer settings. */
static void link_nested(void) {
    compile_consumer();
    char *arguments[26]={TEST_C_COMPILER,"-std=c11","-O1","-g","-Wall","-Wextra","-Werror","-pedantic-errors"};
    size_t count=8;
#ifdef TEST_SANITIZERS
    arguments[count++]="-fsanitize=address,undefined"; arguments[count++]="-fno-omit-frame-pointer";
    arguments[count++]="-fno-pie"; arguments[count++]="-no-pie";
#endif
    arguments[count++]="-I" TEST_SDK; arguments[count++]="-Igenerated";
    arguments[count++]="-Igenerated/parent-output"; arguments[count++]="generated/consumer.o";
    arguments[count++]="generated/parent.c"; arguments[count++]="generated/child-output/library.a";
    arguments[count++]="-o"; arguments[count++]="generated/consumer"; arguments[count]=NULL;
    bool linked=library_package_process(arguments,stderr); assert(linked);
}
