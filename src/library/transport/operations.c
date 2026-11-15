/** @file
 * @brief Implement immutable public snapshot views and explicit independent ownership operations.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) forbids
 * publication on clone failure and empties the sole owner after release.
 */
#include "internal.h"
void library_transport_operations(LibraryHeader *header, const LibrarySchema *schema) {
    const char *name=schema->identity->internal_name, *prefix=header->prefix;
    library_header_print(header,
        "%s_ApiResult %s_view_implementation(const %s_Value *source, %s_Borrowed *output) {\n"
        "    if (source == NULL || output == NULL || !ama_library_code_contains(&%s_code,source,&%s_schema)) { return %s_result((AmaLibraryResult){AMA_LIBRARY_INVALID_REQUEST,ama_status_make(AMA_INVALID_STATE,NULL,0)}); }\n"
        "    *output=source->data; return %s_result((AmaLibraryResult){AMA_LIBRARY_OK,ama_status_success()});\n}\n"
        "%s_ApiResult %s_clone_implementation(const %s_Value *source, %s_Value **output) {\n"
        "    if (source == NULL || output == NULL || *output != NULL || !ama_library_code_contains(&%s_code,source,&%s_schema)) { return %s_result((AmaLibraryResult){AMA_LIBRARY_INVALID_REQUEST,ama_status_make(AMA_INVALID_STATE,NULL,0)}); }\n"
        "    AmaStatus status=%s_copy(&source->data,output);\n"
        "    return %s_result((AmaLibraryResult){status.code == AMA_OK ? AMA_LIBRARY_OK : AMA_LIBRARY_TECHNICAL_FAILURE,status});\n}\n"
        "%s_ApiResult %s_release_implementation(%s_Value **owner) {\n"
        "    if (owner == NULL || (*owner != NULL && !ama_library_code_contains(&%s_code,*owner,&%s_schema))) { return %s_result((AmaLibraryResult){AMA_LIBRARY_INVALID_REQUEST,ama_status_make(AMA_INVALID_STATE,NULL,0)}); }\n"
        "    %s_free(*owner); *owner=NULL; return %s_result((AmaLibraryResult){AMA_LIBRARY_OK,ama_status_success()});\n}\n",
        prefix,name,name,name,prefix,name,prefix,prefix,prefix,name,name,name,prefix,name,prefix,name,prefix,prefix,name,name,prefix,name,prefix,name,prefix);
}
