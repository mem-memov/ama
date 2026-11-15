/** @file
 * @brief Generate independently owned snapshots from the checked wire schema.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) excludes Ama
 * references and continuation storage; these owners contain only copied wire data.
 */
#include <assert.h>
#include "internal.h"
bool library_snapshots_write(const LibraryBuild *build, FILE *output) {
    assert(build->valid);
    if (!library_header_target(build,NULL)) { return false; }
    LibraryHeader header = {.build=build,.output=output,
        .prefix=build->identity->internal_name,.good=true};
    library_header_print(&header,"#include <stdlib.h>\n#include <string.h>\n");
    library_transport_owners(&header);
    library_transport_shapes(&header);
    for (const LibrarySchema *schema=build->schemas; schema!=NULL; schema=schema->next) {
        if (schema->kind==LIBRARY_LIST) { library_transport_list(&header,schema); }
        else { library_transport_scalar(&header,schema); }
        library_transport_operations(&header,schema);
    }
    return header.good && fflush(output)==0;
}
void library_transport_owners(LibraryHeader *header) {
    for (const LibrarySchema *schema=header->build->schemas; schema!=NULL; schema=schema->next) {
        const char *name=schema->identity->internal_name;
        library_header_print(header,"static const char %s_schema;\nstruct %s_Value { AmaLibraryCodeOwner code_owner; %s_Borrowed data;",name,name,name);
        if (schema->kind==LIBRARY_LIST) {
            library_header_print(header," %s_Value **owners;",schema->element->identity->internal_name);
        }
        library_header_print(header," };\nstatic void %s_free(%s_Value *);\n"
            "static AmaStatus %s_copy(const %s_Borrowed *, %s_Value **);\n"
            "static bool %s_shape(const %s_Borrowed *);\n",name,name,name,name,name,name,name);
    }
}
