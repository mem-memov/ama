/** @file
 * @brief Retain verified library artifacts and reclaim every private staging file.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) publishes
 * only complete owned bytes; scratch cleanup never removes caller source files.
 */
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include "internal.h"
bool driver_library_retain(NativeScaffold *scaffold, const char *stage) {
    static const char *const names[]={"public.h","component.c","component.json","symbols.txt","library.a","library.so"};
    for (size_t index=0;index<sizeof(names)/sizeof(*names);index++) {
        const char *source=scaffold_join(scaffold,stage,"/",names[index]);
        const char *destination=scaffold_join(scaffold,scaffold->directory,"/",names[index]);
        if (source==NULL || destination==NULL) { return false; }
        const SourceText *content=NULL;
        if (source_manager_load(&scaffold->texts,source,&content)!=SOURCE_OK) { return false; }
        ScaffoldArtifact *artifact=scaffold_allocate(scaffold,sizeof(*artifact));
        if (artifact==NULL) { return false; }
        *artifact=(ScaffoldArtifact){.path=destination,.content=content};
        scaffold->last->next=artifact; scaffold->last=artifact;
    }
    return true;
}
void driver_library_cleanup(const char *stage) {
    DIR *directory=opendir(stage);
    if (directory!=NULL) {
        struct dirent *entry;
        while ((entry=readdir(directory))!=NULL) {
            if (strcmp(entry->d_name,".")!=0 && strcmp(entry->d_name,"..")!=0) {
                (void)unlinkat(dirfd(directory),entry->d_name,0);
            }
        }
        (void)closedir(directory);
    }
    (void)rmdir(stage);
}
