/** @file
 * @brief Preserve library C/link setting boundaries without shell splitting or guessing.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) keeps
 * every explicit option as one argv element; mandatory backend flags follow user flags.
 */
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "process.h"
#include "native/toolchain/profile.h"
/** Allocate enough zero-terminated argument slots, with checked count/byte arithmetic. */
static char **arguments(const LibraryBuild *build, size_t fixed) {
    size_t count=fixed;
    for (const LibrarySetting *setting=build->settings;setting!=NULL;setting=setting->next) {
        if (count==SIZE_MAX) { return NULL; } count++;
    }
    return count>SIZE_MAX/sizeof(char *) ? NULL : calloc(count,sizeof(char *));
}
bool library_package_compile(const LibraryBuild *build, const LibraryPackageOptions *options, FILE *errors) {
    const char *tail[]={AMA_TOOLCHAIN_REQUIRED_FLAGS,"-fPIC","-fvisibility=default",
        "-I",options->sdk,"-I",options->includes,"-c",options->source,"-o",options->raw_object,NULL};
    char **argv=arguments(build,2+sizeof(tail)/sizeof(*tail));
    if (argv==NULL) { return false; }
    size_t count=0; argv[count++]=(char *)options->compiler; argv[count++]="-O2";
    for (const LibrarySetting *setting=build->settings;setting!=NULL;setting=setting->next) {
        if (strcmp(setting->kind,"c-flag")==0) { argv[count++]=(char *)setting->value; }
    }
    for(size_t i=0;i<sizeof(tail)/sizeof(*tail);i++) { argv[count++]=(char *)tail[i]; }
    bool compiled=library_package_process(argv,errors); free(argv); return compiled;
}
bool library_package_link(const LibraryBuild *build, const LibraryPackageOptions *options, FILE *errors) {
    char **argv=arguments(build,8);
    if (argv==NULL) { return false; }
    size_t count=0; argv[count++]=(char *)options->compiler; argv[count++]="-shared";
    argv[count++]="-fno-lto"; argv[count++]=(char *)options->isolated_object;
    for (const LibrarySetting *setting=build->settings;setting!=NULL;setting=setting->next) {
        if (strcmp(setting->kind,"link-flag")==0 || strcmp(setting->kind,"link-file")==0) {
            argv[count++]=(char *)setting->value;
        }
    }
    argv[count++]="-Wl,-z,defs"; argv[count++]="-o"; argv[count++]=(char *)options->shared;
    bool linked=library_package_process(argv,errors); free(argv); return linked;
}
