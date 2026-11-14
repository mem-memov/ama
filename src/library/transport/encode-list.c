/** @file
 * @brief Encode ordered owning-list elements into independent scalar snapshot owners.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) preserves
 * value order and releases every completed element when a later encoder fails.
 */
#include "codecs.h"
void library_codec_list(LibraryCodecs *codecs, const LibrarySchema *schema) {
    LibraryHeader *writer=&codecs->output;
    const char *name=schema->identity->internal_name, *element=schema->element->identity->internal_name;
    library_header_print(writer,
        "static inline AmaStatus %s_encode(AmaProgram *program,const AmaProgramValue *source,%s_Value **output) {\n"
        "    if (source->form!=AMA_FIELD_OWNING_LIST || source->list==NULL || source->type_id!=UINT64_C(%zu)) { return ama_status_make(AMA_TYPE_MISMATCH,NULL,0); }\n"
        "    size_t count=source->list->count;\n"
        "    if (count>SIZE_MAX/sizeof(%s_Borrowed) || count>SIZE_MAX/sizeof(%s_Value *)) { return ama_status_make(AMA_LIMIT_EXCEEDED,NULL,0); }\n"
        "    %s_Value *snapshot=malloc(sizeof(*snapshot));\n"
        "    if (snapshot==NULL) { return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }\n"
        "    *snapshot=(%s_Value){0};\n"
        "    if (count!=0) {\n"
        "        %s_Borrowed *items=malloc(count*sizeof(*items));\n"
        "        snapshot->owners=malloc(count*sizeof(*snapshot->owners)); snapshot->data.items=items;\n"
        "        if (items==NULL || snapshot->owners==NULL) { %s_free(snapshot); return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }\n"
        "        for (size_t i=0;i<count;i++) {\n"
        "            AmaProgramValue item={.type_id=source->type_id,.form=AMA_FIELD_OWNED,.handle=*ama_handle_list_element(source->list,i)};\n"
        "            AmaStatus status=%s_encode(program,&item,&snapshot->owners[i]);\n"
        "            if (status.code!=AMA_OK) { %s_free(snapshot); return status; }\n"
        "            items[i]=snapshot->owners[i]->data; snapshot->data.length++;\n"
        "        }\n    }\n"
        "    *output=snapshot; return ama_status_success();\n}\n",
        name,name,library_codec_type_id(codecs,schema->type),element,element,name,name,element,name,element,name);
}
