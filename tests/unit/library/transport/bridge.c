/** @file
 * @brief Generate real component code plus test-only entry-preparation and allocator probes.
 */
/** Count all private native/runtime/wire allocations; failure ordinal is relative to selection. */
static void allocator(FILE *output) {
    int written=fputs(
        "#include \"public.h\"\n#include <stdlib.h>\n#include <assert.h>\n"
        "static size_t calls,fail,live;\nvoid select_failure(size_t offset) { calls=0; fail=offset; }\n"
        "size_t allocation_calls(void) { return calls; }\nsize_t live_allocations(void) { return live; }\n"
        "static void *transport_malloc(size_t size) { calls++; if(calls==fail) return NULL; void *p=malloc(size); if(p!=NULL) live++; return p; }\n"
        "static void transport_free(void *p) { if(p!=NULL) { assert(live!=0); live--; free(p); } }\n"
        "#define malloc transport_malloc\n#define free transport_free\n#include \"output/project.c\"\n",output);
    assert(written!=EOF);
}
/** Prepare inputs through ordinary runtime field installation. This probe executes no Ama body;
 * public entry/resume dispatch is T127. It isolates T126's actual generated argument builder.
 */
static void preparation_probe(FILE *output) {
    int written=fputs(
        "API(ApiResult) fixture_prepare(API(Context) context,API(Instance) instance,\n"
        " const TEXT(Borrowed)*text,const NUMBER(Borrowed)*number,const CONDITION(Borrowed)*condition,const FAILURE(Borrowed)*failure,const LIST(Borrowed)*list) {\n"
        " if(!EXPORTED(input_shape)(text,number,condition,failure,list)) return API(result)((AmaLibraryResult){AMA_LIBRARY_INVALID_REQUEST,ama_status_make(AMA_INVALID_STATE,NULL,0)});\n"
        " AmaLibraryAccess access={0}; AmaHandle root={0};\n"
        " AmaLibraryResult result=ama_library_select(&API(service),API(context)(context),API(instance)(instance),1,AMA_ACCESS_ENTRY,&access,&root);\n"
        " if(result.code!=0) return API(result)(result);\n"
        " SourceArena arena; source_arena_init(&arena,access.view.world->allocator);\n"
        " AmaProgramInput bindings[5]; size_t count=0;\n"
        " AmaStatus status=EXPORTED(inputs)(&arena,bindings,&count,text,number,condition,failure,list);\n"
        " if(status.code==0) {\n"
        "  AmaPoolCell *cell=NULL; status=ama_program_cell(access.view.program,root,&cell);\n"
        "  assert(status.code==0); cell->metadata.service.outgoing=root;\n"
        "  cell->metadata.service.outgoing_previous=cell->metadata.service.activity; cell->metadata.service.activity=AMA_INSTANCE_ACQUIRED;\n"
        "  AmaProgramInvocation invocation={.mode=AMA_ACCESS_ENTRY,.inputs=bindings,.input_count=count};\n"
        "  status=ama_program_prepare(access.view.program,root,&invocation);\n"
        "  if(status.code==0) status=ama_program_release(access.view.program,root);\n"
        "  if(status.code==0) {\n"
        "   AmaProgramAccess source={.field=4}, destination={.field=9}; AmaProgramValue value={0};\n"
        "   status=ama_program_select(access.view.program,root,&source,false,&value);\n"
        "   if(status.code==0) status=ama_program_transfer(access.view.program,root,&destination,&value);\n"
        "  }\n"
        " }\n"
        " source_arena_destroy(&arena);\n"
        " if(status.code!=0) result=ama_library_fail_at(&API(service),&access,status,root,0);\n"
        " ama_library_leave(&access); return API(result)(result);\n}\n"
        "void fixture_busy_child(API(Context) context,API(Instance) instance,int busy) {\n"
        " AmaLibraryAccess access={0}; AmaHandle root={0};\n"
        " AmaLibraryResult result=ama_library_select(&API(service),API(context)(context),API(instance)(instance),1,AMA_ACCESS_READ,&access,&root); assert(result.code==0);\n"
        " AmaProgramAccess field={.field=5}; AmaProgramValue value={0};\n"
        " AmaStatus status=ama_program_select(access.view.program,root,&field,false,&value); assert(status.code==0);\n"
        " AmaPoolCell *cell=NULL; status=ama_program_cell(access.view.program,value.handle,&cell); assert(status.code==0);\n"
        " cell->metadata.service.activity=busy ? AMA_INSTANCE_ACQUIRED : AMA_INSTANCE_READY; ama_library_leave(&access);\n}\n"
        "void fixture_yield(API(Context) context,API(Instance) instance) {\n"
        " AmaLibraryAccess access={0}; AmaHandle root={0};\n"
        " AmaLibraryResult result=ama_library_select(&API(service),API(context)(context),API(instance)(instance),1,AMA_ACCESS_READ,&access,&root); assert(result.code==0);\n"
        " AmaPoolCell *cell=NULL; AmaStatus status=ama_program_cell(access.view.program,root,&cell); assert(status.code==0);\n"
        " cell->metadata.service.activity=AMA_INSTANCE_YIELDED; cell->metadata.service.resume=(AmaResumePosition){true,17}; ama_library_leave(&access);\n}\n"
        "void fixture_check_yield(API(Context) context,API(Instance) instance) {\n"
        " AmaLibraryAccess access={0}; AmaHandle root={0};\n"
        " AmaLibraryResult result=ama_library_select(&API(service),API(context)(context),API(instance)(instance),1,AMA_ACCESS_RESUME,&access,&root); assert(result.code==0);\n"
        " AmaPoolCell *cell=NULL; AmaStatus status=ama_program_cell(access.view.program,root,&cell); assert(status.code==0);\n"
        " assert(cell->metadata.service.resume.valid && cell->metadata.service.resume.selector==17); ama_library_leave(&access);\n}\n",output);
    assert(written!=EOF);
}
/** Emit the additional scalar-list fixture after all private codec definitions. */
static void other_lists_probe(FILE *);
/** Compose public ABI, explicit lifecycle, independent owners, typed codecs and test probes. */
static void bridge(const LibraryBuild *build, NativeScaffold *assembly) {
    FILE *output=fopen("generated/component.c","wb"); assert(output!=NULL);
    allocator(output);
    bool written=library_abi_write(build,output); assert(written);
    uint64_t type=codegen_type_id(assembly,native_scaffold_type(assembly,build->exports->type)); assert(type!=0);
    written=library_lifecycle_write(build,&type,1,output); assert(written);
    written=library_snapshots_write(build,output); assert(written);
    written=library_codecs_write(build,assembly,output); assert(written);
    aliases(output,build); preparation_probe(output); other_lists_probe(output);
    int closed=fclose(output); assert(closed==0);
}
/** Exercise the other supported scalar-list products through real declared Ama state fields.
 * This test-only probe owns no persistent native references and keeps every recipe/handle
 * within one admitted synchronous operation. Public result selection remains export-bound.
 */
static void other_lists_probe(FILE *output) {
    int written=fputs(
        "void fixture_other_lists(API(Context) context,API(Instance) instance,const NUMBERS(Borrowed)*numbers,const CONDITIONS(Borrowed)*conditions,const FAILURES(Borrowed)*failures,NUMBERS(Value)**number_output,CONDITIONS(Value)**condition_output,FAILURES(Value)**failure_output) {\n"
        " AmaLibraryAccess access={0}; AmaHandle root={0};\n"
        " AmaLibraryResult admitted=ama_library_select(&API(service),API(context)(context),API(instance)(instance),1,AMA_ACCESS_READ,&access,&root); assert(admitted.code==0);\n"
        " SourceArena arena; source_arena_init(&arena,access.view.world->allocator);\n"
        " AmaFieldRecipe recipes[3];\n"
        " AmaStatus status=NUMBERS(decode)(&arena,numbers,&recipes[0]); assert(status.code==0);\n"
        " status=CONDITIONS(decode)(&arena,conditions,&recipes[1]); assert(status.code==0);\n"
        " status=FAILURES(decode)(&arena,failures,&recipes[2]); assert(status.code==0);\n"
        " AmaProgramValue values[3];\n"
        " AmaObjectOperation operation={.world=access.view.world}; source_arena_init(&operation.arena,operation.world->allocator); operation.world->operation_active=true;\n"
        " for(size_t i=0;i<3;i++) {\n"
        "  AmaProgramAccess field={.field=11+i}; status=ama_program_select(access.view.program,root,&field,false,&values[i]); assert(status.code==0);\n"
        "  status=ama_program_store(access.view.program,&values[i],NULL,&recipes[i],&operation); assert(status.code==0);\n"
        " }\n"
        " ama_object_unprotect(&operation); source_arena_destroy(&operation.arena); operation.world->operation_active=false; source_arena_destroy(&arena);\n"
        " status=NUMBERS(encode)(access.view.program,&values[0],number_output); assert(status.code==0);\n"
        " status=CONDITIONS(encode)(access.view.program,&values[1],condition_output); assert(status.code==0);\n"
        " status=FAILURES(encode)(access.view.program,&values[2],failure_output); assert(status.code==0);\n"
        " ama_library_leave(&access);\n}\n",output);
    assert(written!=EOF);
}
