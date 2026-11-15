/** @file
 * @brief Reject incompatible targets, incomplete tools and surviving entry symbols before packaging.
 */
#include "library/artifact/verify.h"
#include <unistd.h>
/** Give each negative attempt fresh unpublished destinations without changing the real source. */
static LibraryPackageOptions destinations(const LibraryPackageOptions *original, char paths[5][512], const char *suffix) {
    LibraryPackageOptions options=*original;
    const char *names[]={original->raw_object,original->isolated_object,original->symbols,original->archive,original->shared};
    for(size_t i=0;i<5;i++) {
        int size=snprintf(paths[i],512,"%s.%s",names[i],suffix); assert(size>0 && size<512);
    }
    options.raw_object=paths[0]; options.isolated_object=paths[1]; options.symbols=paths[2];
    options.archive=paths[3]; options.shared=paths[4]; return options;
}
/** Assertions use actual ELF objects, including a deliberately surviving registered entry family. */
static void package_rejections(const LibraryBuild *build, const LibraryPackageOptions *original) {
    FILE *errors=tmpfile(); assert(errors!=NULL);
    FILE *assembly=tmpfile(); assert(assembly!=NULL);
    assert(!library_assembly_write(build,original->assembly,"bad\"header.h","project.c",assembly));
    assert(!library_assembly_write(build,original->assembly,"?" "?/header.h","project.c",assembly));
    assert(!library_assembly_write(build,original->assembly,"tab\theader.h","project.c",assembly));
    assert(ftell(assembly)==0);
    int assembly_closed=fclose(assembly); assert(assembly_closed==0);
    /* Existing owned paths refuse before opening/truncating any destination. */
    assert(!library_package_build(build,original,errors));
    char paths[5][512];
    LibraryPackageOptions options=destinations(original,paths,"target");
    LibraryBuild unsupported=*build; unsupported.target="aarch64-linux-gnu";
    assert(!library_package_build(&unsupported,&options,errors));
    assert(access(options.symbols,F_OK)!=0 && errno==ENOENT);
    options.assembly=NULL; assert(!library_package_build(build,&options,errors));
    options=destinations(original,paths,"alias"); options.raw_object=options.source;
    assert(!library_package_build(build,&options,errors));
    assert(access(options.symbols,F_OK)!=0 && errno==ENOENT);
    options=destinations(original,paths,"compiler"); options.compiler="/bin/false";
    assert(!library_package_build(build,&options,errors));
    assert(access(options.archive,F_OK)!=0 && errno==ENOENT);
    FILE *source=fopen(original->source,"ab"); assert(source!=NULL);
    const ScaffoldType *native=original->assembly->types;
    while(native!=NULL && native->plan->type->module->syntax.sections.kind!=MODULE_NATIVE) { native=native->next; }
    assert(native!=NULL);
    int written=fprintf(source,
        "\nvoid surviving_entry(void) __asm__(\"%s_entry.constprop.0\");\nvoid surviving_entry(void) {}\n",
        native->native_prefix->internal_name); assert(written>0);
    int closed=fclose(source); assert(closed==0);
    options=destinations(original,paths,"entry");
    assert(!library_package_build(build,&options,errors));
    assert(access(options.raw_object,F_OK)==0 && access(options.isolated_object,F_OK)==0);
    assert(access(options.archive,F_OK)!=0 && errno==ENOENT);
    assert(access(options.shared,F_OK)!=0 && errno==ENOENT);
    closed=fclose(errors); assert(closed==0);
}
/** Process failures and signals cannot be mistaken for successful tools or invoke a shell implicitly. */
static void it_checks_tool_status_and_exact_captured_argument_bytes(void) {
    char *missing[]={"/definitely/absent/ama-tool",NULL};
    char *failure[]={"/bin/false",NULL};
    char *signal[]={"/bin/sh","-c","kill -TERM $$",NULL};
    assert(!library_package_process(missing,stderr));
    assert(!library_package_process(failure,stderr));
    assert(!library_package_process(signal,stderr));
    FILE *output=tmpfile(); assert(output!=NULL);
    char *bytes[]={"/usr/bin/printf","%s","literal spaces ; $(no expansion)",NULL};
    assert(library_package_capture(bytes,output,stderr));
    int seek=fseek(output,0,SEEK_SET); assert(seek==0);
    char actual[80]={0}; size_t length=fread(actual,1,sizeof(actual)-1,output);
    assert(!ferror(output) && length==strlen(bytes[2]) && strcmp(actual,bytes[2])==0);
    int closed=fclose(output); assert(closed==0);
}
