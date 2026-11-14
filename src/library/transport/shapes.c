/** @file
 * @brief Validate bounded C shapes and discriminants before operation admission.
 * [EMBED-03](../../../spec/implementation/c-library.md#embed-03) reserves
 * UTF-8/canonical-value decoding for the admitted technical-construction phase.
 */
#include "internal.h"
void library_transport_shapes(LibraryHeader *header) {
    for (const LibrarySchema *schema=header->build->schemas; schema!=NULL; schema=schema->next) {
        const char *name=schema->identity->internal_name;
        library_header_print(header,"static bool %s_shape(const %s_Borrowed *value) {\n"
            "    if (value == NULL) { return false; }\n",name,name);
        if (schema->kind==LIBRARY_CONDITION) {
            library_header_print(header,"    return value->value <= UINT8_C(1);\n");
        } else if (schema->kind==LIBRARY_LIST) {
            library_header_print(header,
                "    if ((value->items == NULL && value->length != 0)\n"
                "        || value->length > SIZE_MAX / sizeof(*value->items)\n"
                "        || value->length > SIZE_MAX / sizeof(%s_Value *)) { return false; }\n"
                "    for (size_t i=0; i<value->length; i++) { if (!%s_shape(&value->items[i])) { return false; } }\n"
                "    return true;\n",schema->element->identity->internal_name,schema->element->identity->internal_name);
        } else {
            const char *field=schema->kind==LIBRARY_INTEGER ? "magnitude." : schema->kind==LIBRARY_FAILURE ? "description." : "";
            if (schema->kind==LIBRARY_INTEGER || schema->kind==LIBRARY_FAILURE) {
                library_header_print(header,"    if (value->%s > UINT8_C(1)) { return false; }\n",
                    schema->kind==LIBRARY_INTEGER ? "negative" : "failed");
            }
            library_header_print(header,"    return value->%sdata != NULL || value->%slength == 0;\n",field,field);
        }
        library_header_print(header,"}\n");
    }
}
