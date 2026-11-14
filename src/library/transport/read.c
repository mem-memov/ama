/** @file
 * @brief Read retained results under one executor lease and publish independent owners together.
 * [EMBED-03](../../../spec/implementation/c-library.md#embed-03) requires
 * effect-free admission rejection and complete unpublished snapshot cleanup.
 */
#include "codecs.h"
/** Emit the selected result's exact owned pointer type and ordinal; skip internal fields. */
static const LibrarySchema *result_schema(LibraryHeader *writer, const LibraryExport *exported,
    const SemanticSymbol *field) {
    return library_header_binding(writer->build,exported,field)->schema;
}
/** Validate initialized-empty distinct destinations before touching any context state. */
static void destinations(LibraryHeader *writer, const LibraryExport *exported) {
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_RESULT) { continue; }
        library_header_print(writer,
            "    if (result_%zu!=NULL && *result_%zu!=NULL) { return %s_result((AmaLibraryResult){AMA_LIBRARY_INVALID_REQUEST,ama_status_make(AMA_INVALID_STATE,NULL,0)}); }\n",
            field->role_ordinal,field->role_ordinal,writer->prefix);
        for (const SemanticSymbol *previous=exported->type->first; previous!=field; previous=previous->next) {
            if (previous->role!=SYMBOL_RESULT) { continue; }
            library_header_print(writer,
                "    if (result_%zu!=NULL && (const void *)result_%zu==(const void *)result_%zu) { return %s_result((AmaLibraryResult){AMA_LIBRARY_INVALID_REQUEST,ama_status_make(AMA_INVALID_STATE,NULL,0)}); }\n",
                field->role_ordinal,field->role_ordinal,previous->role_ordinal,writer->prefix);
        }
    }
}
/** Preflight each requested owned result, then release protections before pure typed codecs run.
 * The executor lease excludes intervening Ama execution; no C borrow survives this operation.
 */
static void preflight(LibraryHeader *writer, const LibraryExport *exported) {
    library_header_print(writer,
        "    AmaObjectOperation check={.world=access.view.world}; source_arena_init(&check.arena,check.world->allocator);\n"
        "    check.world->operation_active=true;\n");
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_RESULT) { continue; }
        library_header_print(writer,
            "    if (status.code==AMA_OK && result_%zu!=NULL) {\n"
            "        operation=ama_program_type(access.view.program,target.type_id)->fields[%zu].operation;\n"
            "        AmaProgramAccess field={.field=%zu};\n"
            "        status=ama_program_select(access.view.program,target,&field,false,&value_%zu);\n"
            "        if (status.code==AMA_OK) {\n",
            field->role_ordinal,field->ordinal,field->ordinal,field->role_ordinal);
        if (field->list) {
            library_header_print(writer,
                "            for (size_t i=0; i<value_%zu.list->count && status.code==AMA_OK; i++) {\n"
                "                status=ama_object_preflight(&check,*ama_handle_list_element(value_%zu.list,i),false,false);\n"
                "            }\n",field->role_ordinal,field->role_ordinal);
        } else {
            library_header_print(writer,"            status=ama_object_preflight(&check,value_%zu.handle,false,false);\n",field->role_ordinal);
        }
        library_header_print(writer,"        }\n    }\n");
    }
    library_header_print(writer,
        "    ama_object_unprotect(&check); source_arena_destroy(&check.arena); check.world->operation_active=false;\n");
}
void library_codec_read(LibraryCodecs *codecs, const LibraryExport *exported, size_t identity) {
    LibraryHeader *writer=&codecs->output;
    const char *name=exported->identity->internal_name, *prefix=writer->prefix;
    library_header_print(writer,"%s_ApiResult %s_read_implementation(%s_Context context,%s_Instance instance",prefix,name,prefix,prefix);
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_RESULT) { continue; }
        library_header_print(writer,",%s_Value **result_%zu",result_schema(writer,exported,field)->identity->internal_name,field->role_ordinal);
    }
    library_header_print(writer,") {\n    call_once(&%s_once,%s_setup);\n",prefix,prefix);
    destinations(writer,exported);
    library_header_print(writer,
        "    AmaLibraryAccess access={0}; AmaHandle target={0};\n"
        "    AmaLibraryResult admitted=ama_library_select(&%s_service,%s_context(context),%s_instance(instance),UINT64_C(%zu),AMA_ACCESS_READ,&access,&target);\n"
        "    if (admitted.code!=AMA_LIBRARY_OK) { return %s_result(admitted); }\n",
        prefix,prefix,prefix,identity,prefix);
    if (exported->type->counts[SYMBOL_RESULT]==0) {
        library_header_print(writer,"    ama_library_leave(&access); return %s_result(admitted);\n}\n",prefix); return;
    }
    library_header_print(writer,"    AmaStatus status=ama_status_success();\n"
        "    uint64_t operation=ama_program_type(access.view.program,target.type_id)->operation;\n");
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_RESULT) { continue; }
        library_header_print(writer,"    AmaProgramValue value_%zu={0}; %s_Value *snapshot_%zu=NULL;\n",
            field->role_ordinal,result_schema(writer,exported,field)->identity->internal_name,field->role_ordinal);
    }
    preflight(writer,exported);
    library_header_print(writer,
        "    if (status.code==AMA_BUSY) { ama_library_leave(&access); return %s_result((AmaLibraryResult){AMA_LIBRARY_BUSY,status}); }\n",prefix);
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_RESULT) { continue; }
        library_header_print(writer,
            "    if (status.code==AMA_OK && result_%zu!=NULL) {\n"
            "        operation=ama_program_type(access.view.program,target.type_id)->fields[%zu].operation;\n"
            "        status=%s_encode(access.view.program,&value_%zu,&snapshot_%zu);\n    }\n",
            field->role_ordinal,field->ordinal,result_schema(writer,exported,field)->identity->internal_name,
            field->role_ordinal,field->role_ordinal);
    }
    library_header_print(writer,"    if (status.code!=AMA_OK) {\n");
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_RESULT) { continue; }
        library_header_print(writer,"        %s_free(snapshot_%zu);\n",result_schema(writer,exported,field)->identity->internal_name,field->role_ordinal);
    }
    library_header_print(writer,
        "        AmaLibraryResult failed=ama_library_fail_at(&%s_service,&access,status,target,operation);\n"
        "        ama_library_leave(&access); return %s_result(failed);\n    }\n",prefix,prefix);
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_RESULT) { continue; }
        library_header_print(writer,"    if (result_%zu!=NULL) { *result_%zu=snapshot_%zu; }\n",field->role_ordinal,field->role_ordinal,field->role_ordinal);
    }
    library_header_print(writer,"    ama_library_leave(&access); return %s_result(admitted);\n}\n",prefix);
}
