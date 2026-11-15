/** @file
 * @brief Own independently checked child/parent models and compile their actual C boundary.
 */
#ifndef AMA_NESTED_COMPONENT_TEST_COMMON
#define AMA_NESTED_COMPONENT_TEST_COMMON
#include "../common.c"
#include "codegen/codegen.h"
#include "codegen/internal.h"
#include "library/artifact/assembly.h"
#include "library/artifact/package.h"
#include "library/artifact/process.h"
/** Resolve explicit component records against exactly one own project plus standard authority. */
static void nested_model(LibraryBuild *build, const char *component, const char *root, const char *records) {
    char text[8192];
    const SourceText *original=configuration(records);
    const char *bytes=(const char *)source_bytes(original), *old=strstr(bytes,"example.component");
    assert(old!=NULL);
    int length=snprintf(text,sizeof(text),"%.*s%s%s",(int)(old-bytes),bytes,component,old+strlen("example.component"));
    assert(length>0 && (size_t)length<sizeof(text));
    const SourceText *source=NULL;
    SourceStatus added=source_manager_add(&sources,component,(const unsigned char *)text,(size_t)length,&source);
    assert(added==SOURCE_OK);
    library_build_init(build,sources.allocator);
    bool valid=library_configuration_parse(build,source); assert(valid);
    ModuleRoot roots[]={{.path=root,.kind=MODULE_ROOT_PROJECT},
        {.path=TEST_STANDARD_ROOT,.kind=MODULE_ROOT_STANDARD}};
    valid=library_build_resolve(build,roots,2);
    if (!valid) {
        SourceSpan site=build->error.primary.span;
        fprintf(stderr,"%s at %s:%zu: %s\n",component,site.source==NULL ? "<configuration>" : source_path(site.source),site.start,build->error.message);
    }
    assert(valid);
}
/** Return a declared schema's complete qualified C name. */
static const char *nested_schema(const LibraryBuild *build, const char *name) {
    for(const LibrarySchema *schema=build->schemas;schema!=NULL;schema=schema->next) {
        if(strcmp(schema->name,name)==0) { return schema->identity->internal_name; }
    }
    assert(false); return NULL;
}
/** Publish a consumer header only after its complete checked generation. */
static void nested_header(const LibraryBuild *build, const char *path) {
    FILE *output=fopen(path,"wb"); assert(output!=NULL);
    bool written=library_header_write(build,output);
    int closed=fclose(output); assert(written && closed==0);
}
/** Record generated private authoring names without copying private child layouts into the parent. */
static void authoring_aliases(const ScaffoldType *type, const char *tag, const char *path) {
    FILE *output=fopen(path,"wb"); assert(output!=NULL);
    int written=fprintf(output,
        "#define %s_INTERFACE_HEADER \"%s\"\n#define %s_BEGIN_HEADER \"%s\"\n#define %s_END_HEADER \"%s\"\n"
        "#define %s_PARAMETERS %s\n#define %s_RESULTS %s\n",
        tag,type->interface_name,tag,type->begin_name,tag,type->end_name,
        tag,type->names[SCAFFOLD_PARAMETERS]->internal_name,tag,type->names[SCAFFOLD_RESULTS]->internal_name);
    int closed=fclose(output); assert(written>0 && closed==0);
}
/** Bind public aliases from validated metadata; the included header remains the ABI authority. */
static void public_aliases(const LibraryBuild *build, const char *tag, const char *header, const char *path) {
    FILE *output=fopen(path,"wb"); assert(output!=NULL);
    int written=fprintf(output,
        "#include \"%s\"\n#define %s(n) %s_ ## n\n#define %s_EXPORT(n) %s_ ## n\n"
        "#define %s_TEXT(n) %s_ ## n\n#define %s_CONDITION(n) %s_ ## n\n",
        header,tag,build->identity->internal_name,tag,build->exports->identity->internal_name,
        tag,nested_schema(build,"text"),tag,nested_schema(build,"condition")); assert(written>0);
    char upper[80]; const char *prefix=build->identity->internal_name;
    size_t index=0;
    do { char byte=prefix[index]; upper[index]=byte>='a' && byte<='z' ? (char)(byte-'a'+'A') : byte; } while(prefix[index++]!=0);
    written=fprintf(output,"#define %s_YIELDED %s_YIELDED\n#define %s_FAILED %s_FAILED\n",tag,upper,tag,upper); assert(written>0);
    if(build->exports->next!=NULL) {
        written=fprintf(output,"#define %s_BROKEN(n) %s_ ## n\n",tag,build->exports->next->identity->internal_name); assert(written>0);
    }
    for(const LibrarySchema *schema=build->schemas;schema!=NULL;schema=schema->next) {
        if(strcmp(schema->name,"failure")==0) {
            assert(schema->kind==LIBRARY_FAILURE);
            written=fprintf(output,"#define %s_FAILURE(n) %s_ ## n\n",tag,schema->identity->internal_name); assert(written>0);
        }
    }
    int closed=fclose(output); assert(closed==0);
}
/** Execute one independently linked consumer without any shell or compiler SDK borrow. */
static void nested_execute(const char *path) {
    char *arguments[]={(char *)path,NULL};
    bool success=library_package_process(arguments,stderr); assert(success);
}
#endif
