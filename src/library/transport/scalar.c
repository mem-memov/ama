/** @file
 * @brief Copy scalar bytes into private snapshot allocations without shared native backings.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) permits
 * immutable snapshots across host calls, independently of native context lifetimes.
 */
#include "internal.h"
void library_transport_scalar(LibraryHeader *header, const LibrarySchema *schema) {
    const char *name=schema->identity->internal_name;
    const char *bytes=schema->kind==LIBRARY_INTEGER ? "magnitude." : schema->kind==LIBRARY_FAILURE ? "description." : "";
    library_header_print(header,"static void %s_free(%s_Value *value) {\n"
        "    if (value == NULL) { return; }\n",name,name);
    if (schema->kind!=LIBRARY_CONDITION) {
        library_header_print(header,"    free((void *)value->data.%sdata);\n",bytes);
    }
    library_header_print(header,"    free(value);\n}\nstatic AmaStatus %s_copy(const %s_Borrowed *source, %s_Value **output) {\n"
        "    if (!%s_shape(source)) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }\n"
        "    %s_Value *value = malloc(sizeof(*value));\n"
        "    if (value == NULL) { return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }\n"
        "    *value = (%s_Value){.data=*source};\n",name,name,name,name,name,name);
    if (schema->kind!=LIBRARY_CONDITION) {
        library_header_print(header,
            "    value->data.%sdata = NULL;\n"
            "    if (source->%slength != 0) {\n"
            "        uint8_t *bytes = malloc(source->%slength);\n"
            "        if (bytes == NULL) { %s_free(value); return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }\n"
            "        memcpy(bytes,source->%sdata,source->%slength); value->data.%sdata=bytes;\n    }\n",
            bytes,bytes,bytes,name,bytes,bytes,bytes);
    }
    library_header_print(header,"    *output=value; return ama_status_success();\n}\n");
}
