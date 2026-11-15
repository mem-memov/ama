/** @file
 * @brief Publish list snapshots only after every scalar element owner was constructed.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) requires
 * independent nested ownership and cleanup of an unpublished partial prefix.
 */
#include "internal.h"
void library_transport_list(LibraryHeader *header, const LibrarySchema *schema) {
    const char *name=schema->identity->internal_name;
    const char *element=schema->element->identity->internal_name;
    library_header_print(header,
        "static void %s_free(%s_Value *value) {\n"
        "    if (value == NULL) { return; }\n"
        "    ama_library_code_unregister(&%s_code,&value->code_owner);\n"
        "    while (value->data.length != 0) { %s_free(value->owners[--value->data.length]); }\n"
        "    free(value->owners); free((void *)value->data.items); free(value);\n}\n"
        "static AmaStatus %s_copy(const %s_Borrowed *source, %s_Value **output) {\n"
        "    if (!%s_shape(source)) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }\n"
        "    %s_Value *value = malloc(sizeof(*value));\n"
        "    if (value == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }\n"
        "    *value=(%s_Value){0};\n"
        "    if (source->length != 0) {\n"
        "        %s_Borrowed *items=malloc(source->length*sizeof(*items));\n"
        "        value->owners=malloc(source->length*sizeof(*value->owners));\n"
        "        value->data.items=items;\n"
        "        if (items == NULL || value->owners == NULL) { %s_free(value); return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }\n"
        "        for (size_t i=0; i<source->length; i++) {\n"
        "            AmaStatus status=%s_copy(&source->items[i],&value->owners[i]);\n"
        "            if (status.code != AMA_OK) { %s_free(value); return status; }\n"
        "            items[i]=value->owners[i]->data; value->data.length++;\n"
        "        }\n    }\n"
        "    if (!ama_library_code_register(&%s_code,&value->code_owner,&%s_schema)) { %s_free(value); return ama_status_make(AMA_LIMIT_EXCEEDED,NULL,0); }\n"
        "    *output=value; return ama_status_success();\n}\n",
        name,name,header->prefix,element,name,name,name,name,name,name,element,name,element,name,header->prefix,name,name);
}
