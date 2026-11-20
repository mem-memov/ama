/** @file
 * @brief Own address-sensitive native counter fixtures used through actual public C exports.
 */
/** Define a native root whose body state and native resource survive repeated entry resets. */
static void counter_sources(void) {
    file("generated/project/packages/test/fixture/v1.0.0/src/counter","@<\nmessage\n//text\n |factory\n@>\nflag\n//std/condition\n@C\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/counter.abi","+ initialize\n- destroy\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/counter.h",
        "/** Test-only root owns a native resource and remembers its final address. */\n"
        "struct AMA_NATIVE(State) { void *resource; const void *address; size_t calls; bool fail_destroy; };\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/counter.c","/* Fixture body is completed after checked nominal C names exist. */\n");
}
/** Emit one native entry using the generated exact parameter/result views and standard text API. */
static void counter_body(NativeScaffold *assembly) {
    const SemanticType *semantic=semantic_find_type(&assembly->frontend->semantic,"/test/fixture/v1.0.0/src/counter");
    const ScaffoldType *type=native_scaffold_type(assembly,semantic);
    const ScaffoldType *text=native_scaffold_type(assembly,semantic_find_type(&assembly->frontend->semantic,"//text"));
    assert(type!=NULL && text!=NULL);
    FILE *output=fopen(type->body_path,"wb"); assert(output!=NULL);
    int written=fprintf(output,
        "#include \"%s\"\n#include \"%s\"\n#include <assert.h>\n#include <stdlib.h>\n#include <string.h>\n"
        "extern void fixture_reenter(void);\n"
        "AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) {\n"
        " *state=(AMA_NATIVE(State)){.resource=malloc(1),.address=state};\n"
        " return state->resource==NULL ? ama_status_make(AMA_OUT_OF_MEMORY,NULL,0) : ama_status_success();\n}\n"
        "AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {\n"
        " assert(state->address==state && state->resource!=NULL); free(state->resource); state->resource=NULL;\n"
        " return state->fail_destroy ? ama_status_make(AMA_BUSY,\"counter cleanup failed\",sizeof(\"counter cleanup failed\")-1) : ama_status_success();\n}\n"
        "AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,const %s *parameters,%s *results,const AmaNativeServices *services,AmaStatus *status) {\n"
        " (void)services; assert(state->address==state && state->resource!=NULL);\n"
        " if(state->calls==SIZE_MAX) { *status=ama_status_make(AMA_LIMIT_EXCEEDED,NULL,0); return; }\n"
        " state->calls++; fixture_reenter();\n"
        " AmaByteView message={0}; *status=%s_view(parameters->AMA_INPUT(0)->payload,&message);\n"
        " if(status->code!=0) return;\n"
        " if(message.length==sizeof(\"error-cleanup\")-1 && memcmp(message.data,\"error-cleanup\",message.length)==0) {\n"
        "  state->fail_destroy=true; *status=ama_status_make(AMA_NATIVE_FAILURE,\"counter body failed\",sizeof(\"counter body failed\")-1); return;\n }\n"
        " if(message.length==sizeof(\"error\")-1 && memcmp(message.data,\"error\",message.length)==0) {\n"
        "  *status=ama_status_make(AMA_NATIVE_FAILURE,\"counter body failed\",sizeof(\"counter body failed\")-1); return;\n }\n"
        " results->AMA_OUTPUT(0)->payload->value=state->calls%%2; *status=ama_status_success();\n}\n"
        "#include \"%s\"\n",
        type->interface_name,type->begin_name,type->names[SCAFFOLD_PARAMETERS]->internal_name,
        type->names[SCAFFOLD_RESULTS]->internal_name,text->native_prefix->internal_name,type->end_name);
    int closed=fclose(output); assert(written>0 && closed==0);
}
