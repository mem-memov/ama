/**
 * @file
 * @brief Emit constants/root startup and explicit entry restart preparation.
 *
 * [RT-11](../../../../spec/implementation/runtime.md#rt-11)
 * Initialize once, preserve partial startup for terminal cleanup and never treat resume as entry reinitialization.
 */
#include "codegen/internal.h"

/**
 * Write first startup and explicit root entry restart. Mark initialized before fallible setup so
 * terminal disposal can unwind; resume bypasses interface preparation.
 */
bool codegen_emit_host_initialize(NativeScaffold *scaffold, FILE *output) {
    return fprintf(output,
            "if(!host->initialized) {\n"
            "    host->initialized=true;\n"
            "    ama_world=(AmaWorld){.types=ama_project_registry,\n"
            "        .type_count=sizeof(ama_project_registry)/sizeof(ama_project_registry[0]),.allocator=source_system_allocator()};\n"
            "    ama_program=(AmaProgram){.world=&ama_world,.types=ama_project_program_types,\n"
            "        .type_count=sizeof(ama_project_program_types)/sizeof(ama_project_program_types[0]),\n"
            "        .constants=ama_project_constants,.constant_count=%zu,\n"
            "        .locations=ama_project_locations,.location_count=sizeof(ama_project_locations)/sizeof(ama_project_locations[0])};\n"
            "    ama_world.report=ama_program_report; ama_world.report_context=&ama_program;\n"
            "    ama_status=ama_project_prepare_types(&ama_world);\n"
            "    if(ama_status.code==AMA_OK) ama_status=ama_program_start(&ama_program,UINT64_C(%zu),&ama_root);\n"
            "    host->root=ama_root; ama_current=ama_root;\n"
            "    if(ama_status.code!=AMA_OK) goto ama_project_finish;\n"
            "} else if(mode==AMA_ACCESS_ENTRY) {\n"
            "    ama_status=ama_program_cell(&ama_program,ama_root,&ama_cell);\n"
            "    if(ama_status.code==AMA_OK) ama_status=ama_service_access(&ama_cell->metadata.service,mode);\n"
            "    if(ama_status.code!=AMA_OK) goto ama_project_finish;\n"
            "    ama_cell->metadata.service.outgoing=ama_root;\n"
            "    ama_cell->metadata.service.outgoing_previous=ama_cell->metadata.service.activity;\n"
            "    ama_cell->metadata.service.activity=AMA_INSTANCE_ACQUIRED;\n"
            "    ama_status=ama_program_prepare(&ama_program,ama_root,&(AmaProgramInvocation){.mode=AMA_ACCESS_ENTRY});\n"
            "    ama_cell->metadata.service.outgoing=(AmaHandle){0};\n"
            "    if(ama_status.code!=AMA_OK) goto ama_project_finish;\n"
            "}\n",
        scaffold->frontend->semantic.constant_count, codegen_type_id(scaffold, scaffold->selected)) >= 0;
}
