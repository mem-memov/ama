/** @file
 * @brief Generate typed scalar profile construction and native-to-wire conversion.
 * [EMBED-04](../../../spec/implementation/c-library.md#embed-04) uses exact
 * standard module contracts; no arbitrary payload layout is inferred or serialized.
 */
#include "codecs.h"
/** Emit one fresh scalar decoder; every supplied pointer is a synchronous validated C borrow. */
static void decoder(LibraryCodecs *codecs, const LibrarySchema *schema) {
    LibraryHeader *writer=&codecs->output;
    const char *name=schema->identity->internal_name;
    const ScaffoldType *type=library_codec_type(codecs,schema->type);
    const char *native=type->native_prefix->internal_name;
    const char *adapter=type->names[SCAFFOLD_WRITE]->internal_name;
    library_header_print(writer,"static inline AmaStatus %s_native_decode(void *payload,const void *borrow) {\n"
        "    const %s_Borrowed *input=borrow;\n",name,name);
    if (schema->kind==LIBRARY_INTEGER) {
        library_header_print(writer,"    return %s_from_magnitude((%s_State *)payload,input->negative!=0,\n"
            "        (AmaByteView){input->magnitude.data,input->magnitude.length});\n",native,native);
    } else if (schema->kind==LIBRARY_TEXT) {
        library_header_print(writer,"    return %s_Literal(payload,(AmaByteView){input->data,input->length});\n",adapter);
    } else if (schema->kind==LIBRARY_CONDITION) {
        library_header_print(writer,"    const unsigned char byte=(unsigned char)('0'+input->value);\n"
            "    return %s_Literal(payload,(AmaByteView){&byte,1});\n",adapter);
    } else {
        library_header_print(writer,
            "    if (input->failed==0 && input->description.length!=0) { return ama_status_make(AMA_INVALID_LITERAL,NULL,0); }\n"
            "    AmaStatus status=%s_Default(payload);\n"
            "    if (status.code==AMA_OK) { ((%s_State *)payload)->failed=input->failed!=0; }\n"
            "    return status;\n",adapter,native);
    }
    library_header_print(writer,"}\n");
}
/** Encode arbitrary precision directly into owned big-endian bytes with no intermediate copy. */
static void integer_encoder(LibraryHeader *writer, const char *name, const char *native) {
    library_header_print(writer,
        "    const %s_State *state=payload;\n"
        "    if (!state->live || state->limbs==NULL || state->count==0\n"
        "        || (state->count>1 && state->limbs[state->count-1]==0)\n"
        "        || (state->negative && state->count==1 && state->limbs[0]==0)) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }\n"
        "    %s_Value *snapshot=malloc(sizeof(*snapshot));\n"
        "    if (snapshot==NULL) { return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }\n"
        "    uint8_t *bytes=malloc(state->count);\n"
        "    if (bytes==NULL) { free(snapshot); return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }\n"
        "    for (size_t i=0;i<state->count;i++) { bytes[i]=state->limbs[state->count-i-1]; }\n"
        "    *snapshot=(%s_Value){.data={state->negative ? UINT8_C(1) : UINT8_C(0),{bytes,state->count}}};\n"
        "    *output=snapshot; return ama_status_success();\n",native,name,name);
}
/** Read one failure's owned text field through its exact generated descriptor and typed text API. */
static void failure_encoder(LibraryCodecs *codecs, const LibrarySchema *schema) {
    LibraryHeader *writer=&codecs->output;
    const char *name=schema->identity->internal_name;
    const ScaffoldType *text=library_codec_type(codecs,schema->type->first->type);
    const char *text_native=text->native_prefix->internal_name;
    library_header_print(writer,
        "    bool failed=false;\n"
        "    status=ama_status_checked(%s_Condition(payload,&failed));\n"
        "    if (status.code!=AMA_OK) { return status; }\n"
        "    AmaProgramAccess field={.field=%zu}; AmaProgramValue description={0};\n"
        "    status=ama_program_select(program,value->handle,&field,false,&description);\n"
        "    if (status.code!=AMA_OK) { return status; }\n"
        "    const void *text_payload=NULL;\n"
        "    status=ama_program_payload(program,&description,&text_payload);\n"
        "    if (status.code!=AMA_OK) { return status; }\n"
        "    AmaByteView bytes={0};\n"
        "    status=ama_status_checked(%s_view((const %s_State *)text_payload,&bytes));\n"
        "    if (status.code!=AMA_OK) { return status; }\n"
        "    if (!failed && bytes.length!=0) { return ama_status_make(AMA_INVALID_STATE,NULL,0); }\n"
        "    %s_Borrowed borrow={failed ? UINT8_C(1) : UINT8_C(0),{bytes.data,bytes.length}};\n"
        "    return %s_copy(&borrow,output);\n",
        library_codec_type(codecs,schema->type)->names[SCAFFOLD_WRITE]->internal_name,schema->type->first->ordinal,
        text_native,text_native,name,name);
}
void library_codec_scalar(LibraryCodecs *codecs, const LibrarySchema *schema) {
    decoder(codecs,schema);
    LibraryHeader *writer=&codecs->output;
    const char *name=schema->identity->internal_name;
    const ScaffoldType *type=library_codec_type(codecs,schema->type);
    const char *native=type->native_prefix->internal_name;
    library_header_print(writer,
        "static inline AmaStatus %s_encode(AmaProgram *program,const AmaProgramValue *value,%s_Value **output) {\n"
        "    if (value->type_id!=UINT64_C(%zu) || value->form!=AMA_FIELD_OWNED) { return ama_status_make(AMA_TYPE_MISMATCH,NULL,0); }\n"
        "    const void *payload=NULL; AmaStatus status=ama_program_payload(program,value,&payload);\n"
        "    if (status.code!=AMA_OK) { return status; }\n",
        name,name,library_codec_type_id(codecs,schema->type));
    if (schema->kind==LIBRARY_INTEGER) { integer_encoder(writer,name,native); }
    else if (schema->kind==LIBRARY_FAILURE) { failure_encoder(codecs,schema); }
    else if (schema->kind==LIBRARY_TEXT) {
        library_header_print(writer,
            "    AmaByteView bytes={0}; status=ama_status_checked(%s_view((const %s_State *)payload,&bytes));\n"
            "    if (status.code!=AMA_OK) { return status; }\n"
            "    %s_Borrowed borrow={bytes.data,bytes.length}; return %s_copy(&borrow,output);\n",native,native,name,name);
    } else {
        library_header_print(writer,
            "    bool condition=false; status=ama_status_checked(%s_Condition(payload,&condition));\n"
            "    if (status.code!=AMA_OK) { return status; }\n"
            "    %s_Borrowed borrow={condition ? UINT8_C(1) : UINT8_C(0)}; return %s_copy(&borrow,output);\n",
            type->names[SCAFFOLD_WRITE]->internal_name,name,name);
    }
    library_header_print(writer,"}\n");
}
