/** @file
 * @brief Emit explicit typed scalar bindings for one data-only application schema.
 * Module result fields stay unavailable until completion; technical failure lets runtime cleanup own them.
 */
/** Encode data from bounded typed borrows, then replace an ordinary owned bytes result. */
static bool message_entry(FILE *output, const ScaffoldType *type) {
    const char *path = type->plan->type->module->source->logical_path;
    if (strcmp(path, "/test/fixture/v1.0.0/src/decode") == 0 || strcmp(path, "/test/fixture/v1.0.0/src/inspect") == 0 || strcmp(path, "/test/fixture/v1.0.0/src/inspectfailure") == 0) {
        const char *owner = strcmp(path, "/test/fixture/v1.0.0/src/inspectfailure") == 0 ? "//failure" : "/test/fixture/v1.0.0/src/message";
        const char *const aliases[] = {"TEST_MESSAGE_FLAG", "TEST_MESSAGE_TEXT", "TEST_MESSAGE_NUMBER"};
        bool found = false;
        for (const ScaffoldType *record = assembly.types; record != NULL; record = record->next) {
            if (strcmp(record->plan->type->module->source->logical_path, owner) != 0) { continue; }
            found = true;
            for (const ScaffoldMember *member = record->members; member != NULL; member = member->next) {
                const SemanticSymbol *symbol = member->field->symbol;
                assert(symbol->role == SYMBOL_RESULT && symbol->role_ordinal < 3);
                int written = fprintf(output, "#define %s %s\n", strcmp(owner, "//failure") == 0
                    ? "TEST_FAILURE_DESCRIPTION" : aliases[symbol->role_ordinal], member->identity->internal_name);
                assert(written > 0);
            }
        }
        assert(found);
    }
    const char *body = NULL;
    if (strcmp(path, "/test/fixture/v1.0.0/src/encode") == 0) {
        body =
            "AmaByteView text; *status=ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(parameters->AMA_INPUT(1)->payload,&text);\n"
            "if(status->code!=AMA_OK) return;\n"
            "const ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_State *integer=parameters->AMA_INPUT(2)->payload;\n"
            "unsigned char *magnitude=malloc(integer->count);\n"
            "if(magnitude==NULL) { *status=ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); return; }\n"
            "for(size_t i=0;i<integer->count;i++) magnitude[i]=integer->limbs[integer->count-i-1];\n"
            "AmaNativeMessageValue values[]={{parameters->AMA_INPUT(0)->payload->value,{NULL,0}},\n"
            "{0,text},{integer->negative ? 1u : 0u,{magnitude,integer->count}}};\n"
            "AmaNativeMessageFieldValue fields[]={{&values[0],1},{&values[1],1},{&values[2],1}};\n"
            "AmaNativeMessageBytes wire={0};\n"
            "AmaNativeMessageResult result=ama_native_message_encode(&test_message_schema,fields,&services->code,&wire); free(magnitude);\n"
            "*status=result.status; if(status->code==AMA_OK && result.reason!=AMA_MESSAGE_ACCEPTED) *status=ama_status_make(AMA_INVALID_STATE,NULL,0);\n"
            "if(status->code==AMA_OK) *status=ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b_replace(results->AMA_OUTPUT(0)->payload,(AmaByteView){wire.data,wire.length});\n"
            "ama_native_message_bytes_drop(&wire); if(status->code!=AMA_OK) return;\n";
    } else if (strcmp(path, "/test/fixture/v1.0.0/src/decode") == 0) {
        body =
            "AmaByteView wire; *status=ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b_view(parameters->AMA_INPUT(0)->payload,&wire);\n"
            "if(status->code!=AMA_OK) return;\n"
            "AmaNativeMessageDecoded decoded={0}; AmaNativeMessageResult result=ama_native_message_decode(&test_message_schema,wire,&services->code,&decoded);\n"
            "*status=result.status; if(status->code!=AMA_OK) return;\n"
            "if(result.reason!=AMA_MESSAGE_ACCEPTED) {\n"
            "static const unsigned char message[]=\"Message rejected\";\n"
            "*status=ama_n_edcb3e348eb3d3a06b885e12e77fcb6bd0b86d265d2077a038d757a4f799f515_set(results->AMA_OUTPUT(1),(AmaByteView){message,sizeof(message)-1}); return; }\n"
            "results->AMA_OUTPUT(0)->TEST_MESSAGE_FLAG->payload->value=decoded.fields[0].items[0].flag;\n"
            "*status=ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_replace(results->AMA_OUTPUT(0)->TEST_MESSAGE_TEXT->payload,decoded.fields[1].items[0].bytes);\n"
            "if(status->code==AMA_OK && parameters->AMA_INPUT(1)->payload->value!=0) {\n"
            "*status=ama_status_make(AMA_NATIVE_FAILURE,\"Typed receive failed\",20); }\n"
            "if(status->code==AMA_OK) {\n"
            "ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_State number;\n"
            "*status=ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_from_magnitude(&number,decoded.fields[2].items[0].flag!=0,decoded.fields[2].items[0].bytes);\n"
            "if(status->code==AMA_OK) {\n"
            "*status=ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_destroy(results->AMA_OUTPUT(0)->TEST_MESSAGE_NUMBER->payload);\n"
            "if(status->code==AMA_OK) *results->AMA_OUTPUT(0)->TEST_MESSAGE_NUMBER->payload=number;\n"
            "else (void)ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_destroy(&number); } }\n"
            "ama_native_message_decoded_drop(&decoded); if(status->code!=AMA_OK) return;\n";
    } else if (strcmp(path, "/test/fixture/v1.0.0/src/inspect") == 0) {
        body =
            "AmaByteView text; *status=ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(parameters->AMA_INPUT(0)->TEST_MESSAGE_TEXT->payload,&text);\n"
            "if(status->code!=AMA_OK) return;\n"
            "static const unsigned char expected[]=\"Hello, Ама\";\n"
            "const ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_State *number=parameters->AMA_INPUT(0)->TEST_MESSAGE_NUMBER->payload;\n"
            "if(parameters->AMA_INPUT(0)->TEST_MESSAGE_FLAG->payload->value!=1 || text.length!=sizeof(expected)-1 || memcmp(text.data,expected,text.length)!=0\n"
            "|| !number->negative || number->count!=9 || number->limbs[0]!=1 || number->limbs[8]!=1) { *status=ama_status_make(AMA_INVALID_STATE,NULL,0); return; }\n"
            "for(size_t i=1;i<8;i++) if(number->limbs[i]!=0) { *status=ama_status_make(AMA_INVALID_STATE,NULL,0); return; }\n"
            "puts(\"decoded\");\n";
    } else if (strcmp(path, "/test/fixture/v1.0.0/src/inspectfailure") == 0) {
        body =
            "AmaByteView text; *status=ama_n_762bf961da08fb6cb5c3d7150457175890b330d62849df81b283e451413eef10_view(parameters->AMA_INPUT(0)->TEST_FAILURE_DESCRIPTION->payload,&text);\n"
            "if(status->code!=AMA_OK) return;\n"
            "static const unsigned char expected[]=\"Message rejected\";\n"
            "if(!parameters->AMA_INPUT(0)->payload->failed || text.length!=sizeof(expected)-1 || memcmp(text.data,expected,text.length)!=0) { *status=ama_status_make(AMA_INVALID_STATE,NULL,0); return; }\n"
            "puts(\"rejected\");\n";
    }
    if (body == NULL) { return false; }
    int written = fputs(body, output);
    assert(written != EOF);
    written = fputs("#undef TEST_MESSAGE_FLAG\n#undef TEST_MESSAGE_TEXT\n#undef TEST_MESSAGE_NUMBER\n#undef TEST_FAILURE_DESCRIPTION\n", output);
    assert(written != EOF);
    return true;
}
