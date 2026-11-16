/** @file
 * @brief Emit explicit typed host arguments with shape checks separate from admitted decoding.
 * [EMBED-03](../../../spec/implementation/c-library.md#embed-03) preserves
 * state on admission rejection; [CALL-08](../../../spec/language/calls-and-results.md#call-08)
 * constructs supplied parameters and then default results in declaration order.
 */
#include "codecs.h"
/** Emit one exact input's checked C type and declaration-order name. Component contract: [EMBED-03](../../../spec/implementation/c-library.md#embed-03), [CALL-08](../../../spec/language/calls-and-results.md#call-08). */
static void arguments(LibraryCodecs *codecs, const LibraryExport *exported, bool first) {
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_PARAMETER) { continue; }
        const LibraryBinding *binding=library_header_binding(codecs->output.build,exported,field);
        library_header_print(&codecs->output,"%sconst %s_Borrowed *input_%zu",first ? "" : ",",
            binding->schema->identity->internal_name,field->role_ordinal);
        first=false;
    }
    if (first) { library_header_print(&codecs->output,"void"); }
}
void library_codec_inputs(LibraryCodecs *codecs, const LibraryExport *exported) {
    LibraryHeader *writer=&codecs->output;
    const char *name=exported->identity->internal_name;
    library_header_print(writer,"static inline bool %s_input_shape(",name);
    arguments(codecs,exported,true); library_header_print(writer,") {\n");
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_PARAMETER) { continue; }
        const LibraryBinding *binding=library_header_binding(writer->build,exported,field);
        library_header_print(writer,"    if (input_%zu!=NULL && !%s_shape(input_%zu)) { return false; }\n",
            field->role_ordinal,binding->schema->identity->internal_name,field->role_ordinal);
    }
    library_header_print(writer,"    return true;\n}\nstatic inline AmaStatus %s_inputs(SourceArena *arena,AmaProgramInput *bindings,size_t *count",name);
    if (exported->type->counts[SYMBOL_PARAMETER]!=0) { library_header_print(writer,","); arguments(codecs,exported,true); }
    library_header_print(writer,") {\n    (void)arena; (void)bindings; *count=0;\n");
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_PARAMETER) { continue; }
        const LibraryBinding *binding=library_header_binding(writer->build,exported,field);
        library_header_print(writer,
            "    if (input_%zu!=NULL) {\n"
            "        AmaFieldRecipe *recipe=source_arena_allocate(arena,sizeof(*recipe));\n"
            "        if (recipe==NULL) { return ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }\n"
            "        AmaStatus status=%s_decode(arena,input_%zu,recipe);\n"
            "        if (status.code!=AMA_OK) { return status; }\n"
            "        bindings[(*count)++]=(AmaProgramInput){.field=%zu,\n"
            "            .source={.kind=AMA_SOURCE_PREPARED,.type_id=UINT64_C(%zu),.prepared=recipe}};\n    }\n",
            field->role_ordinal,binding->schema->identity->internal_name,field->role_ordinal,
            library_codec_field_slot(codecs,field),library_codec_type_id(codecs,field->type));
    }
    library_header_print(writer,"    return ama_status_success();\n}\n");
}
