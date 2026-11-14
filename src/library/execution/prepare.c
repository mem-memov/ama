/** @file
 * @brief Prepare typed C inputs and declaration-site trace operands after successful admission.
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08) resets interfaces
 * in declaration order; [EMBED-03](../../../spec/implementation/c-library.md#embed-03)
 * makes allocation/decoding failures after admission terminal, not API refusals.
 */
#include "internal.h"
void library_execution_inputs(LibraryCodecs *codecs, const LibraryExport *exported, bool names) {
    for (const SemanticSymbol *field=exported->type->first; field!=NULL; field=field->next) {
        if (field->role!=SYMBOL_PARAMETER) { continue; }
        if (names) { library_header_print(&codecs->output,",input_%zu",field->role_ordinal); }
        else {
            const LibraryBinding *binding=library_header_binding(codecs->output.build,exported,field);
            library_header_print(&codecs->output,",const %s_Borrowed *input_%zu",binding->schema->identity->internal_name,field->role_ordinal);
        }
    }
}
void library_execution_prepare(LibraryCodecs *codecs, const LibraryExport *exported) {
    LibraryHeader *writer=&codecs->output;
    const char *prefix=writer->prefix, *name=exported->identity->internal_name;
    library_header_print(writer,
        "    SourceArena inputs; source_arena_init(&inputs,access.view.world->allocator);\n"
        "    const AmaProgramType *type=ama_program_type(access.view.program,target.type_id);\n"
        "    AmaStatus preparation=ama_status_success();\n"
        "    uint64_t *operations=NULL; AmaProgramInput *bindings=NULL;\n"
        "    size_t capacity=%zu,count=0;\n"
        "    if (type->field_count>SIZE_MAX/sizeof(*operations) || capacity>SIZE_MAX/sizeof(*bindings)) { preparation=ama_status_make(AMA_LIMIT_EXCEEDED,NULL,0); }\n"
        "    else {\n"
        "        if (type->field_count!=0) { operations=source_arena_allocate(&inputs,type->field_count*sizeof(*operations)); }\n"
        "        if (capacity!=0) { bindings=source_arena_allocate(&inputs,capacity*sizeof(*bindings)); }\n"
        "        if ((type->field_count!=0 && operations==NULL) || (capacity!=0 && bindings==NULL)) { preparation=ama_status_make(AMA_OUT_OF_MEMORY,NULL,0); }\n"
        "    }\n"
        "    if (preparation.code==AMA_OK) {\n"
        "        for (size_t i=0;i<type->field_count;i++) { operations[i]=type->fields[i].operation; }\n"
        "        preparation=%s_inputs(&inputs,bindings,&count",
        exported->type->counts[SYMBOL_PARAMETER],name);
    library_execution_inputs(codecs,exported,true);
    library_header_print(writer,");\n    }\n"
        "    if (preparation.code!=AMA_OK) {\n"
        "        source_arena_destroy(&inputs);\n"
        "        AmaLibraryResult failed=ama_library_fail_at(&%s_service,&access,preparation,target,type->operation);\n"
        "        *outcome=UINT32_C(3); ama_library_leave(&access);\n"
        "        return %s_result(ama_library_result(AMA_LIBRARY_OK,failed.status));\n    }\n"
        "    AmaProgramInvocation invocation={.mode=AMA_ACCESS_ENTRY,.inputs=bindings,.input_count=count,.installation_operations=operations};\n",
        prefix,prefix);
}
