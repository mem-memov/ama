/** @file
 * @brief Emit typed native resource/lifecycle bodies with mandatory inline entry and real parameter-view alignment assertions.
 */
/** Repeated helper names remain module-qualified; every allocation is owned at its final State address and released exactly once. */
static void write_layout_native(const ScaffoldType *type) {
    FILE *output = fopen(type->body_path, "wb");
    assert(output != NULL);
    bool included = scaffold_include(output, type->interface_name) && scaffold_include(output, type->begin_name);
    assert(included);
    int written = fputs("#include <assert.h>\n#include <stdlib.h>\n#include <string.h>\n"
        "static size_t AMA_NATIVE(defaults), AMA_NATIVE(literals), AMA_NATIVE(clones), AMA_NATIVE(destroys), AMA_NATIVE(entries);\n"
        "static AmaStatus AMA_NATIVE(construct)(AMA_NATIVE(State) *state) {\n"
        "assert((uintptr_t)state % _Alignof(AMA_NATIVE(State)) == 0);\n"
        "state->address=(uintptr_t)state; memset(state->marker,0xA5,sizeof(state->marker));\n"
        "state->resource=malloc(13); state->number=0;\n"
        "return state->resource==NULL ? ama_status_make(AMA_OUT_OF_MEMORY,NULL,0) : ama_status_success(); }\n"
        "AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { AMA_NATIVE(defaults)++; return AMA_NATIVE(construct)(state); }\n"
        "AmaStatus AMA_NATIVE(literal)(AMA_NATIVE(State) *state,AmaByteView bytes) {\n"
        "AMA_NATIVE(literals)++; AmaStatus status=AMA_NATIVE(construct)(state); if(status.code==AMA_OK && bytes.length!=0) state->number=bytes.data[0]; return status; }\n"
        "AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *state,const AMA_NATIVE(State) *source) {\n"
        "assert(source->address==(uintptr_t)source); AMA_NATIVE(clones)++;\n"
        "AmaStatus status=AMA_NATIVE(construct)(state); if(status.code==AMA_OK) state->number=source->number; return status; }\n"
        "AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {\n"
        "assert(state->address==(uintptr_t)state && state->resource!=NULL && state->marker[16]==0xA5);\n"
        "AMA_NATIVE(destroys)++; free(state->resource); state->resource=NULL; return ama_status_success(); }\n", output);
    assert(written != EOF);
    written = fprintf(output,
        "AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,const %s *parameters,%s *results,const AmaNativeServices *services,AmaStatus *status) {\n"
        "assert(state->address==(uintptr_t)state && (uintptr_t)state%%64==0);\n"
        "AMA_NATIVE(entries)++; (void)parameters; (void)results; (void)services;\n",
        type->names[SCAFFOLD_PARAMETERS]->internal_name, type->names[SCAFFOLD_RESULTS]->internal_name);
    assert(written > 0);
    if (strcmp(type->plan->type->module->source->logical_path, "/test/fixture/v1.0.0/src/worker") == 0) {
        written = fputs("assert(parameters->AMA_INPUT(0)->payload->address==(uintptr_t)parameters->AMA_INPUT(0)->payload);\n"
            "assert((uintptr_t)parameters->AMA_INPUT(0)->payload%64==0 && parameters->AMA_INPUT(0)->payload->number==65);\n", output);
        assert(written != EOF);
    }
    written = fputs("*status=ama_status_success(); }\n", output);
    assert(written != EOF);
    included = scaffold_include(output, type->end_name);
    int closed = fclose(output);
    assert(included && closed == 0);
}
