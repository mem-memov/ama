/** @file
 * @brief Build real component fixtures with separately owned native text closures.
 */
#ifndef AMA_LIBRARY_PACKAGE_TEST_COMMON
#define AMA_LIBRARY_PACKAGE_TEST_COMMON
#include "../common.c"
#include "codegen/codegen.h"
#include "library/artifact/assembly.h"
#include "library/artifact/package.h"
#include "library/artifact/process.h"
/** Exact generated public names retained after the checked model is destroyed. */
typedef struct { char component[80], exported[80], text[80]; } PackageNames;
/** Optional assertions borrow the real checked inputs before fixture teardown. */
typedef void (*PackageProbe)(const LibraryBuild *, const LibraryPackageOptions *);
/** Publish this fixture's independently consumable public header. */
static void header(const LibraryBuild *build, const char *path) {
    FILE *output=fopen(path,"wb"); assert(output!=NULL);
    bool written=library_header_write(build,output);
    int closed=fclose(output); assert(written && closed==0);
}
/** Execute one host with a checked child status and no command interpreter. */
static void execute(const char *path) {
    char *arguments[]={(char *)path,NULL};
    bool executed=library_package_process(arguments,stderr); assert(executed);
}
/** Build a native-text component, keeping all staging paths separate. */
static void package_component(const char *name, const char *directory_name, const char *root, PackageNames *names,
    bool release, PackageProbe probe) {
    char directory_path[256];
    int size=snprintf(directory_path,sizeof(directory_path),"generated/%s",directory_name); assert(size>0 && (size_t)size<sizeof(directory_path));
    directory(directory_path);
    char exports[1024];
    size=snprintf(exports,sizeof(exports),"schema | text | text | ..text\nexport | run | %s | entry,resume,read\noutput | run | value | text\n"
        ,root); assert(size>0 && (size_t)size<sizeof(exports));
    char settings[2048];
    size=snprintf(settings,sizeof(settings),"%s%s",exports,
        "c-flag | -fvisibility=hidden\nc-flag | -DAMA_COMPONENT_MARKER=7\n"
#ifdef TEST_SANITIZERS
        "c-flag | -fsanitize=address,undefined\nc-flag | -fno-omit-frame-pointer\nlink-flag | -fsanitize=address,undefined\n"
#endif
        ); assert(size>0 && (size_t)size<sizeof(settings));
    const SourceText *original=configuration(settings);
    const char *bytes=(const char *)source_bytes(original), *old=strstr(bytes,"example.component"); assert(old!=NULL);
    char records_buffer[4096];
    size=snprintf(records_buffer,sizeof(records_buffer),"%.*s%s%s%s",(int)(old-bytes),bytes,name,
        old+strlen("example.component"),release ? "c-flag | -DNDEBUG\n" : ""); assert(size>0 && (size_t)size<sizeof(records_buffer));
    const SourceText *source=NULL;
    SourceStatus added=source_manager_add(&sources,name,(const unsigned char *)records_buffer,(size_t)size,&source); assert(added==SOURCE_OK);
    LibraryBuild build; library_build_init(&build,sources.allocator);
    bool valid=library_configuration_parse(&build,source); assert(valid);
    ModuleRoot roots[]={{.path="generated/project",.kind=MODULE_ROOT_PROJECT},
        {.path=TEST_STANDARD_ROOT,.kind=MODULE_ROOT_STANDARD}};
    valid=library_build_resolve(&build,roots,2);
    if (!valid) { fprintf(stderr,"%s\n",build.error.message); }
    assert(valid);
    strcpy(names->component,build.identity->internal_name); strcpy(names->exported,build.exports->identity->internal_name);
    strcpy(names->text,build.schemas->identity->internal_name);
    NativeScaffold assembly; native_scaffold_init(&assembly,&build.frontend,sources.allocator);
    bool built=project_codegen_build(&assembly,(ScaffoldOptions){.directory=directory_path,.sdk=TEST_SDK}); assert(built);
    bool published=native_scaffold_publish(&assembly); assert(published);
    char paths[7][256]; const char *filenames[]={"public.h","component.c","raw.o","isolated.o","symbols.txt","library.a","library.so"};
    for(size_t i=0;i<7;i++) { size=snprintf(paths[i],sizeof(paths[i]),"%s/%s",directory_path,filenames[i]); assert(size>0 && (size_t)size<sizeof(paths[i])); }
    header(&build,paths[0]);
    FILE *output=fopen(paths[1],"wb"); assert(output!=NULL);
    int allocator=fputs(
        "#include <stdlib.h>\n#include <stdbool.h>\n"
        "static bool fail_domain_once=true;\n"
        "static void *domain_calloc(size_t count,size_t size) {\n"
        " if(fail_domain_once) { fail_domain_once=false; return NULL; } return calloc(count,size); }\n"
        "static bool fail_pin_once=true;\n"
        "static void *component_malloc(size_t size) {\n"
        " if(fail_pin_once) { fail_pin_once=false; return NULL; } return malloc(size); }\n"
        "#define malloc component_malloc\n#define calloc domain_calloc\n",output); assert(allocator!=EOF);
    bool written=library_assembly_write(&build,&assembly,"public.h","project.c",output); assert(written);
    allocator=fputs("\n#undef malloc\n#undef calloc\n",output); assert(allocator!=EOF);
    int marked=fputs("\n_Static_assert(AMA_COMPONENT_MARKER==7,\"declared compiler flags must reach the component\");\n",output); assert(marked!=EOF);
    int closed=fclose(output); assert(closed==0);
    LibraryPackageOptions options={.assembly=&assembly,.compiler=TEST_C_COMPILER,.archiver="ar",.object_copy="objcopy",
        .source=paths[1],.sdk=TEST_SDK,.includes=directory_path,.raw_object=paths[2],
        .isolated_object=paths[3],.symbols=paths[4],.archive=paths[5],.shared=paths[6]};
    built=library_package_build(&build,&options,stderr); assert(built);
    if (probe!=NULL) { probe(&build,&options); }
    native_scaffold_destroy(&assembly); library_build_destroy(&build);
}
#endif
