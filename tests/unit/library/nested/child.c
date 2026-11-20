/** @file
 * @brief Package a real isolated child closure with normal and failed-constructor exports.
 */
/** The explicit child constants/defaults initialize before a factory returns, without running bodies. */
static void build_child(LibraryBuild *build, NativeScaffold *assembly) {
    nested_model(build,"nested.child","child/project",
        "schema | text | text | //text\nschema | condition | condition | //std/condition\n"
        "export | run | /test/fixture/v1.0.0/src/main | entry,resume,read\ninput | run | request | text\n"
        "output | run | response | text\noutput | run | previous | condition\nexport | broken | /test/fixture/v1.0.0/src/broken | entry\n"
#ifdef TEST_SANITIZERS
        "c-flag | -fsanitize=address,undefined\nc-flag | -fno-omit-frame-pointer\nlink-flag | -fsanitize=address,undefined\n"
#endif
        );
    directory("generated/child-output");
    native_scaffold_init(assembly,&build->frontend,sources.allocator);
    bool built=project_codegen_build(assembly,(ScaffoldOptions){.directory="generated/child-output",.sdk=TEST_SDK});
    if(!built) { fprintf(stderr,"%s\n",assembly->error.message); } assert(built);
    const ScaffoldType *broken=native_scaffold_type(assembly,semantic_find_type(&build->frontend.semantic,"/test/fixture/v1.0.0/src/broken")); assert(broken!=NULL);
    authoring_aliases(broken,"AMA_BROKEN","generated/child-output/broken-api.h");
    bool published=native_scaffold_publish(assembly); assert(published);
    nested_header(build,"generated/child-output/public.h");
    public_aliases(build,"CHILD","child-output/public.h","generated/child-api.h");
    FILE *output=fopen("generated/child-output/component.c","wb"); assert(output!=NULL);
    bool written=library_assembly_write(build,assembly,"public.h","project.c",output);
    int closed=fclose(output); assert(written && closed==0);
    LibraryPackageOptions options={.assembly=assembly,.compiler=TEST_C_COMPILER,.archiver="ar",.object_copy="objcopy",
        .source="generated/child-output/component.c",.sdk=TEST_SDK,.includes="generated/child-output",
        .raw_object="generated/child-output/raw.o",.isolated_object="generated/child-output/isolated.o",
        .symbols="generated/child-output/symbols.txt",.archive="generated/child-output/library.a",.shared="generated/child-output/library.so"};
    built=library_package_build(build,&options,stderr); assert(built);
}
