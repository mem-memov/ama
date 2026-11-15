/** @file
 * @brief Stage complete library sources without publishing executable launchers.
 * [EMBED-01](../../../../spec/implementation/c-library.md#embed-01) keeps
 * component construction separate from an executable main/default instance.
 */
#include <string.h>
#include "internal.h"
#include "library/artifact/assembly.h"
/** Remove only the exact two executable-only filenames; retain all native declarations. */
static void remove_launcher(NativeScaffold *scaffold) {
    ScaffoldArtifact **link=&scaffold->first;
    scaffold->last=NULL;
    while (*link != NULL) {
        ScaffoldArtifact *artifact=*link;
        const char *name=strrchr(artifact->path,'/'); name=name==NULL ? artifact->path : name+1;
        if (strcmp(name,"launcher.c")==0 || strcmp(name,"compile_commands.json")==0) { *link=artifact->next; }
        else { scaffold->last=artifact; link=&artifact->next; }
    }
}
/** Write one complete generated source, checking close even after a failed writer. */
static bool boundary(NativeScaffold *scaffold, const LibraryBuild *build, const char *stage, unsigned part) {
    static const char *const names[]={"public.h","component.c","component.json"};
    const char *path=scaffold_join(scaffold,stage,"/",names[part]);
    FILE *output=path==NULL ? NULL : fopen(path,"wb");
    if (output==NULL) { return false; }
    bool written=part==0 ? library_header_write(build,output)
        : part==1 ? library_assembly_write(build,scaffold,"public.h","project.c",output)
        : library_manifest_write(build,output);
    int closed=fclose(output);
    return written && closed==0;
}
bool driver_library_stage(NativeScaffold *scaffold, const LibraryBuild *build, const char *stage) {
    remove_launcher(scaffold);
    for (const ScaffoldArtifact *artifact=scaffold->first; artifact!=NULL; artifact=artifact->next) {
        const char *name=strrchr(artifact->path,'/'); name=name==NULL ? artifact->path : name+1;
        const char *path=scaffold_join(scaffold,stage,"/",name);
        FILE *output=path==NULL ? NULL : fopen(path,"wb");
        if (output==NULL) { return false; }
        size_t length=source_length(artifact->content);
        bool written=fwrite(source_bytes(artifact->content),1,length,output)==length;
        int closed=fclose(output);
        if (!written || closed!=0) { return false; }
    }
    return boundary(scaffold,build,stage,0) && boundary(scaffold,build,stage,1)
        && boundary(scaffold,build,stage,2);
}
