/** @file
 * @brief Build arena-owned constructor recipes while borrowing inputs only through entry setup.
 * [EMBED-03](../../../spec/implementation/c-library.md#embed-03) places
 * semantic decoding after admission. [CALL-08](../../../spec/language/calls-and-results.md#call-08)
 * installs exactly one selected constructor in declaration order.
 */
#include "codecs.h"
void library_codec_recipes(LibraryCodecs *codecs, const LibrarySchema *schema) {
    LibraryHeader *writer=&codecs->output;
    const char *name=schema->identity->internal_name;
    if (schema->kind!=LIBRARY_LIST) {
        library_header_print(writer,"static inline AmaStatus %s_recipe(SourceArena *arena,const %s_Borrowed *input,AmaRecipe *output) {\n",name,name);
        if (schema->kind==LIBRARY_FAILURE) {
            library_header_print(writer,
                "    AmaFieldRecipe *fields=source_arena_allocate(arena,sizeof(*fields));\n"
                "    AmaRecipe *text=source_arena_allocate(arena,sizeof(*text));\n"
                "    if (fields==NULL || text==NULL) { return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }\n"
                "    *text=(AmaRecipe){.constructor=AMA_CONSTRUCT_LITERAL,.literal={input->description.data,input->description.length}};\n"
                "    *fields=(AmaFieldRecipe){.count=1,.children=text,.supplied=true};\n"
                "    *output=(AmaRecipe){.constructor=AMA_CONSTRUCT_TRANSPORT,.from_transport=%s_native_decode,\n"
                "        .transport_input=input,.fields=fields,.field_count=1};\n",name);
        } else {
            library_header_print(writer,
                "    (void)arena; *output=(AmaRecipe){.constructor=AMA_CONSTRUCT_TRANSPORT,\n"
                "        .from_transport=%s_native_decode,.transport_input=input};\n",name);
        }
        library_header_print(writer,"    return ama_status_success();\n}\n");
    }
    library_header_print(writer,"static inline AmaStatus %s_decode(SourceArena *arena,const %s_Borrowed *input,AmaFieldRecipe *output) {\n"
        "    if (!%s_shape(input)) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }\n",name,name,name);
    if (schema->kind==LIBRARY_LIST) {
        library_header_print(writer,
            "    if (input->length>SIZE_MAX/sizeof(AmaRecipe)) { return ama_status_make(AMA_LIMIT_EXCEEDED,NULL,0); }\n"
            "    AmaRecipe *recipes=input->length==0 ? NULL : source_arena_allocate(arena,input->length*sizeof(*recipes));\n"
            "    if (input->length!=0 && recipes==NULL) { return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }\n"
            "    for (size_t i=0;i<input->length;i++) {\n"
            "        AmaStatus status=%s_recipe(arena,&input->items[i],&recipes[i]);\n"
            "        if (status.code!=AMA_OK) { return status; }\n    }\n"
            "    *output=(AmaFieldRecipe){.count=input->length,.children=recipes,.supplied=true};\n",
            schema->element->identity->internal_name);
    } else {
        library_header_print(writer,
            "    AmaRecipe *recipe=source_arena_allocate(arena,sizeof(*recipe));\n"
            "    if (recipe==NULL) { return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }\n"
            "    AmaStatus status=%s_recipe(arena,input,recipe);\n"
            "    if (status.code!=AMA_OK) { return status; }\n"
            "    *output=(AmaFieldRecipe){.count=1,.children=recipe,.supplied=true};\n",name);
    }
    library_header_print(writer,"    return ama_status_success();\n}\n");
}
