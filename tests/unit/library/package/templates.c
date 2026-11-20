/** @file
 * @brief Compose concrete C facades with closed static exports and independent template captures.
 */
/** One ordinary source specializes independently to native text and integer Types;
 * the static assembly adds no public/runtime field or instance. */
static void prepare_static_facades(void) {
    directory("generated/project");
    file("generated/project/forward", "@<\ninput\n...element\n@>\noutput\n...element\n input\n@!\noutput\n input\n");
    file("generated/project/assembly", "@...\n...texts\n.forward\n ...element < ..text\n...integers\n.forward\n ...element < ..integer\n");
    file("generated/project/textfacade", "@>\nvalue\n..text\n |hello\n@!\nworker\n.assembly\n ...texts\nworker !\n < value\n > value\n%\n");
    file("generated/project/integerfacade", "@>\nvalue\n..text\n |hello\n@!\nworker\n.assembly\n ...integers\nnumber\n..integer\n |123456789012345678901234567890\nwrong\n..text\n |wrong\nworker !\n < number\n > retained\nbefore\n..integer.before\nbefore !\n < retained\n < number\n > different\ndifferent\n?\n value\n  wrong\nbefore !\n < number\n < retained\n > different\ndifferent\n?\n value\n  wrong\n%\n");
}
/** Inspect the actually packaged model, not a reconstructed mock export list. */
static void facade_package_contract(const LibraryBuild *build, const LibraryPackageOptions *options) {
    assert(build->valid && options->assembly->frontend == &build->frontend);
    assert(build->export_count == 1 && build->exports->type->counts[SYMBOL_RESULT] == 1);
    assert(build->bindings != NULL && build->bindings->next == NULL);
    assert(semantic_find_type(&build->frontend.semantic,".assembly") == NULL);
    size_t workers=0;
    const ScaffoldType *text_worker=NULL,*integer_worker=NULL;
    for (const SemanticType *type=build->frontend.semantic.first;type;type=type->next) {
        if (strcmp(type->module->source->logical_path,".forward") == 0) {
            assert(type->specialization != NULL);
            const SemanticSymbol *input=type->first;
            assert(input != NULL && input->role == SYMBOL_PARAMETER && input->type != NULL);
            const ScaffoldType *worker=native_scaffold_type(options->assembly,type); assert(worker != NULL);
            if (strcmp(input->type->module->source->logical_path,"..text")==0) { assert(text_worker==NULL); text_worker=worker; }
            else { assert(strcmp(input->type->module->source->logical_path,"..integer")==0 && integer_worker==NULL); integer_worker=worker; }
            workers++;
        }
    }
    assert(workers == 2 && text_worker != NULL && integer_worker != NULL);
    assert(strcmp(text_worker->native_prefix->internal_name,integer_worker->native_prefix->internal_name)!=0);
    FILE *manifest=tmpfile(); assert(manifest != NULL);
    assert(library_manifest_write(build,manifest));
    assert(fseek(manifest,0,SEEK_SET)==0);
    char bytes[16384]; size_t count=fread(bytes,1,sizeof(bytes)-1,manifest);
    assert(count < sizeof(bytes)-1 && !ferror(manifest)); bytes[count]=0;
    assert(strstr(bytes,build->exports->identity->internal_name) != NULL);
    assert(fclose(manifest)==0);
}
/** Keep existing exact tool/ABI refusal cases on the actual generic facade. */
static void facade_package_rejections(const LibraryBuild *build,const LibraryPackageOptions *options) {
    facade_package_contract(build,options);
    package_rejections(build,options);
}
