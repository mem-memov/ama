/** @file
 * @brief Compose one parent native adapter against an independently packaged child archive.
 */
/** Native parent construction owns no implicit child body and no native clone capability. */
static void build_parent(LibraryBuild *build, NativeScaffold *assembly) {
    nested_model(build,"nested.parent","project",
        "schema | text | text | //text\nschema | condition | condition | //std/condition\n"
        "export | run | /test/fixture/v1.0.0/src/adapter | entry,read\ninput | run | message | text\n"
        "output | run | response | text\noutput | run | previous | condition\noutput | run | waiting | condition\n"
        "link-file | generated/child-output/library.a\n");
    directory("generated/parent-output");
    native_scaffold_init(assembly,&build->frontend,sources.allocator);
    bool built=project_codegen_build(assembly,(ScaffoldOptions){.directory="generated/parent-output",.sdk=TEST_SDK});
    if(!built) { fprintf(stderr,"%s\n",assembly->error.message); } assert(built);
    const ScaffoldType *adapter=native_scaffold_type(assembly,semantic_find_type(&build->frontend.semantic,"/test/fixture/v1.0.0/src/adapter"));
    const ScaffoldType *text=native_scaffold_type(assembly,semantic_find_type(&build->frontend.semantic,"//text"));
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
