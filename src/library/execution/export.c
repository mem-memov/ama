/** @file
 * @brief Expose only selected entry/resume modes with separate admission and execution outcomes.
 * [EMBED-03](../../../spec/implementation/c-library.md#embed-03) preserves
 * rejected output slots/outcomes and turns admitted execution/transport errors into failed outcomes.
 */
#include "internal.h"
/** Emit one explicit operation's exact public signature and pre-state output/input checks. */
static void operation(LibraryCodecs *codecs, const LibraryExport *exported, size_t identity, bool resume) {
    LibraryHeader *writer=&codecs->output;
    const char *prefix=writer->prefix, *name=exported->identity->internal_name;
    library_header_print(writer,"%s_ApiResult %s_%s_implementation(%s_Context context,%s_Instance instance",
        prefix,name,resume ? "resume" : "entry",prefix,prefix);
    if (!resume) { library_execution_inputs(codecs,exported,false); }
    library_codec_result_arguments(codecs,exported,false);
    library_header_print(writer,",uint32_t *outcome) {\n    call_once(&%s_once,%s_setup);\n"
        "    if (outcome==NULL || !%s_output_shape(0",prefix,prefix,name);
    library_codec_result_arguments(codecs,exported,true);
    library_header_print(writer,")) { return %s_result((AmaLibraryResult){AMA_LIBRARY_INVALID_REQUEST,ama_status_make(AMA_INVALID_STATE,NULL,0)}); }\n",prefix);
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_RESULT) { continue; }
        library_header_print(writer,
            "    if (%s_overlap(outcome,sizeof(*outcome),result_%zu,sizeof(*result_%zu))) { return %s_result((AmaLibraryResult){AMA_LIBRARY_INVALID_REQUEST,ama_status_make(AMA_INVALID_STATE,NULL,0)}); }\n",
            prefix,field->role_ordinal,field->role_ordinal,prefix);
    }
    if (!resume) {
        library_header_print(writer,"    if (!%s_input_shape(",name);
        bool first=true;
        for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
            if (field->role!=SYMBOL_PARAMETER) { continue; }
            library_header_print(writer,"%sinput_%zu",first ? "" : ",",field->role_ordinal); first=false;
        }
        library_header_print(writer,")) { return %s_result((AmaLibraryResult){AMA_LIBRARY_INVALID_REQUEST,ama_status_make(AMA_INVALID_STATE,NULL,0)}); }\n",prefix);
    }
    library_header_print(writer,
        "    AmaLibraryAccess access={0}; AmaHandle target={0};\n"
        "    AmaLibraryResult admitted=ama_library_select(&%s_service,%s_context(context),%s_instance(instance),UINT64_C(%zu),%s,&access,&target);\n"
        "    if (admitted.code!=AMA_LIBRARY_OK) { return %s_result(admitted); }\n",
        prefix,prefix,prefix,identity,resume ? "AMA_ACCESS_RESUME" : "AMA_ACCESS_ENTRY",prefix);
    if (!resume) { library_execution_prepare(codecs,exported); }
    else { library_header_print(writer,"    AmaProgramInvocation invocation={.mode=AMA_ACCESS_RESUME};\n"); }
    library_header_print(writer,
        "    AmaHostOutcome completed=AMA_HOST_FAILED;\n"
        "    AmaLibraryResult executed=%s_run(&access,target,&invocation,%s,&completed);\n"
        "    if (executed.code==AMA_LIBRARY_OK && completed!=AMA_HOST_FAILED) {\n"
        "        AmaLibraryResult snapshots=%s_outputs(&access,target",
        prefix,resume ? "NULL" : "&inputs",name);
    library_codec_result_arguments(codecs,exported,true);
    library_header_print(writer,");\n"
        "        if (snapshots.code!=AMA_LIBRARY_OK) {\n"
        "            if (snapshots.code!=AMA_LIBRARY_TECHNICAL_FAILURE) { snapshots=ama_library_fail_at(&%s_service,&access,snapshots.status,target,ama_program_type(access.view.program,target.type_id)->operation); }\n"
        "            executed=ama_library_result(AMA_LIBRARY_OK,snapshots.status); completed=AMA_HOST_FAILED;\n"
        "        }\n    }\n"
        "    if (executed.code==AMA_LIBRARY_OK) { *outcome=completed==AMA_HOST_COMPLETED ? UINT32_C(1) : completed==AMA_HOST_YIELDED ? UINT32_C(2) : UINT32_C(3); }\n"
        "    ama_library_leave(&access); return %s_result(executed);\n}\n",prefix,prefix);
}
void library_execution_export(LibraryCodecs *codecs, const LibraryExport *exported, size_t identity) {
    if ((exported->operations & LIBRARY_ENTRY)!=0) { operation(codecs,exported,identity,false); }
    if ((exported->operations & LIBRARY_RESUME)!=0) { operation(codecs,exported,identity,true); }
}
