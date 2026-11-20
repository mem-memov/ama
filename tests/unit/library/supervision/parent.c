/** @file
 * @brief Compose an outer Ama loop with a native adapter and independently compiled child.
 */
/** Declared failure owns its description in Ama storage; the payload owns only foreign controls. */
static void build_supervision_parent(LibraryBuild *build,NativeScaffold *assembly) {
    nested_model(build,"supervision.parent","project",
        "schema | text | text | //text\nschema | condition | condition | //std/condition\nschema | failure | failure | //failure\n"
        "export | run | /test/fixture/v1.0.0/src/main | entry,resume,read\ninput | run | message | text\ninput | run | reset | condition\ninput | run | propagate | condition\n"
        "output | run | response | text\noutput | run | previous | condition\noutput | run | waiting | condition\noutput | run | failure | failure\n"
        "link-file | generated/child-output/library.a\nlink-file | generated/probe.o\n");
    directory("generated/parent-output"); native_scaffold_init(assembly,&build->frontend,sources.allocator);
    bool built=project_codegen_build(assembly,(ScaffoldOptions){.directory="generated/parent-output",.sdk=TEST_SDK});
    if(!built) { fprintf(stderr,"%s\n",assembly->error.message); } assert(built);
    const ScaffoldType *adapter=native_scaffold_type(assembly,semantic_find_type(&build->frontend.semantic,"/test/fixture/v1.0.0/src/adapter"));
    const ScaffoldType *text=native_scaffold_type(assembly,semantic_find_type(&build->frontend.semantic,"//text"));
    const ScaffoldType *failure=native_scaffold_type(assembly,semantic_find_type(&build->frontend.semantic,"//failure"));
    assert(adapter!=NULL && text!=NULL && failure!=NULL);
    authoring_aliases(adapter,"AMA_EMBED","generated/adapter-api.h");
    FILE *aliases=fopen("generated/adapter-api.h","ab"); assert(aliases!=NULL);
    int written=fprintf(aliases,"#include \"child-api.h\"\n#define AMA_EMBED_TEXT(n) %s_ ## n\n#define AMA_EMBED_FAILURE(n) %s_ ## n\n",
        text->native_prefix->internal_name,failure->native_prefix->internal_name);
    int closed=fclose(aliases); assert(written>0 && closed==0);
    bool published=native_scaffold_publish(assembly); assert(published);
    nested_header(build,"generated/parent-output/public.h");
    public_aliases(build,"PARENT","parent-output/public.h","generated/parent-api.h");
    FILE *output=fopen("generated/parent.c","wb"); assert(output!=NULL); parent_probe(output);
    bool emitted=library_assembly_write(build,assembly,"parent-output/public.h","parent-output/project.c",output);
    closed=fclose(output); assert(emitted && closed==0);
}
