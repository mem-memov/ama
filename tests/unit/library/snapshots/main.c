/** @file
 * @brief Compile generated wire owners with a separately compiled public-header-only C consumer.
 */
#include "../header/common.c"
#include "library/lifecycle/internal.h"
/** Write test allocator controls before the generated snapshot implementation. */
static void allocator(FILE *output) {
    int written=fputs(
        "#include \"public.h\"\n#include <stdlib.h>\n#include <assert.h>\n"
        "#include \"runtime/library/lifecycle.h\"\n"
        "#include \"runtime/library/code.c\"\n"
        "static size_t calls, fail, live;\n"
        "void select_failure(size_t offset) { calls=0; fail=offset; }\n"
        "size_t allocation_calls(void) { return calls; }\nsize_t live_allocations(void) { return live; }\n"
        "static void *snapshot_malloc(size_t size) { calls++; if(calls==fail) return NULL; void *p=malloc(size); if(p!=NULL) live++; return p; }\n"
        "static void snapshot_free(void *p) { if(p!=NULL) { assert(live!=0); live--; free(p); } }\n"
        "#define malloc snapshot_malloc\n#define free snapshot_free\n",output);
    assert(written!=EOF);
}
/** Obtain the exact named schema identity from the checked model, with no basename inference. */
static const char *schema_name(const LibraryBuild *build, const char *name) {
    for (const LibrarySchema *schema=build->schemas; schema!=NULL; schema=schema->next) {
        if (strcmp(schema->name,name)==0) { return schema->identity->internal_name; }
    }
    assert(false); return NULL;
}
/** Fixture adapters create owners through generated copy code, then invalidate source borrows. */
static void fixtures(FILE *output, const LibraryBuild *build) {
    const char *text=schema_name(build,"text"), *number=schema_name(build,"number");
    const char *condition=schema_name(build,"enabled"), *failure=schema_name(build,"failure");
    const char *list=schema_name(build,"texts");
    int written=fprintf(output,
        "int make_values(%s_Value **text, %s_Value **number, %s_Value **condition, %s_Value **failure, %s_Value **list) {\n"
        " uint8_t bytes[]={97,0,208,176}; uint8_t magnitude[]={1,0,254};\n"
        " %s_Borrowed t={bytes,sizeof(bytes)}; %s_Borrowed n={1,{magnitude,sizeof(magnitude)}};\n"
        " %s_Borrowed c={1}; %s_Borrowed f={1,{bytes,sizeof(bytes)}};\n"
        " %s_Borrowed elements[]={t,t}; %s_Borrowed l={elements,2};\n"
        " AmaStatus status=%s_copy(&t,text); if(status.code!=0) return 1;\n"
        " status=%s_copy(&n,number); if(status.code!=0) return 1;\n"
        " status=%s_copy(&c,condition); if(status.code!=0) return 1;\n"
        " status=%s_copy(&f,failure); if(status.code!=0) return 1;\n"
        " status=%s_copy(&l,list); if(status.code!=0) return 1;\n"
        " memset(bytes,0,sizeof(bytes)); memset(magnitude,0,sizeof(magnitude)); return 0;\n}\n"
        "void check_shapes(void) {\n"
        " %s_Borrowed t={NULL,1}; assert(!%s_shape(&t)); t.length=0; assert(%s_shape(&t));\n"
        " %s_Borrowed n={2,{NULL,0}}; assert(!%s_shape(&n));\n"
        " %s_Borrowed c={2}; assert(!%s_shape(&c));\n"
        " %s_Borrowed f={2,{NULL,0}}; assert(!%s_shape(&f));\n"
        " %s_Borrowed l={NULL,1}; assert(!%s_shape(&l));\n"
        " l.items=&t; l.length=SIZE_MAX; assert(!%s_shape(&l));\n}\n",
        text,number,condition,failure,list,text,number,condition,failure,text,list,
        text,number,condition,failure,list,text,text,text,number,number,condition,condition,failure,failure,list,list,list);
    assert(written>0);
}
/** Emit a C consumer that never includes the compiler SDK or opaque owner definitions. */
static void consumer(const LibraryBuild *build) {
    FILE *output=fopen("generated/consumer.c","wb"); assert(output!=NULL);
    int written=fprintf(output,
        "#include \"public.h\"\n#include <assert.h>\n#include <string.h>\n"
        "#define API(name) %s_ ## name\n#define TEXT(name) %s_ ## name\n#define NUMBER(name) %s_ ## name\n"
        "#define CONDITION(name) %s_ ## name\n#define FAILURE(name) %s_ ## name\n#define LIST(name) %s_ ## name\n",
        build->identity->internal_name,schema_name(build,"text"),schema_name(build,"number"),
        schema_name(build,"enabled"),schema_name(build,"failure"),schema_name(build,"texts"));
    assert(written>0);
    written=fputs(
        "extern void select_failure(size_t); extern size_t allocation_calls(void),live_allocations(void); extern void check_shapes(void);\n"
        "extern int make_values(TEXT(Value)**,NUMBER(Value)**,CONDITION(Value)**,FAILURE(Value)**,LIST(Value)**);\n"
        "static void ok(API(ApiResult) result) { assert(result.code==0); assert(API(result_release)(&result)==0); }\n"
        "int main(void) {\n"
        " TEXT(Value) *text=NULL; NUMBER(Value) *number=NULL; CONDITION(Value) *condition=NULL; FAILURE(Value) *failure=NULL; LIST(Value) *list=NULL;\n"
        " check_shapes(); assert(make_values(&text,&number,&condition,&failure,&list)==0);\n"
        " const uint8_t expected[]={97,0,208,176}; const uint8_t magnitude[]={1,0,254};\n"
        " TEXT(Borrowed) t={0}; NUMBER(Borrowed) n={0}; CONDITION(Borrowed) c={0}; FAILURE(Borrowed) f={0}; LIST(Borrowed) l={0};\n"
        " ok(TEXT(view)(text,&t)); assert(t.length==sizeof(expected) && memcmp(t.data,expected,sizeof(expected))==0);\n"
        " ok(NUMBER(view)(number,&n)); assert(n.negative==1 && n.magnitude.length==sizeof(magnitude) && memcmp(n.magnitude.data,magnitude,sizeof(magnitude))==0);\n"
        " ok(CONDITION(view)(condition,&c)); assert(c.value==1);\n"
        " ok(FAILURE(view)(failure,&f)); assert(f.failed==1 && f.description.length==sizeof(expected) && memcmp(f.description.data,expected,sizeof(expected))==0);\n"
        " ok(LIST(view)(list,&l)); assert(l.length==2 && l.items[0].data!=l.items[1].data);\n"
        " TEXT(Value) *text_copy=NULL; NUMBER(Value) *number_copy=NULL; CONDITION(Value) *condition_copy=NULL; FAILURE(Value) *failure_copy=NULL; LIST(Value) *list_copy=NULL;\n"
        " ok(TEXT(clone)(text,&text_copy)); ok(NUMBER(clone)(number,&number_copy)); ok(CONDITION(clone)(condition,&condition_copy)); ok(FAILURE(clone)(failure,&failure_copy));\n"
        " select_failure(0); ok(LIST(clone)(list,&list_copy)); size_t count=allocation_calls(); assert(count!=0); ok(LIST(release)(&list_copy));\n"
        " size_t retained=live_allocations();\n"
        " for(size_t i=1;i<=count;i++) { select_failure(i); API(ApiResult) result=LIST(clone)(list,&list_copy); assert(result.code==5 && list_copy==NULL && live_allocations()==retained); assert(API(result_release)(&result)==0); }\n"
        " select_failure(0); ok(LIST(clone)(list,&list_copy));\n"
        " API(ApiResult) invalid=TEXT(clone)(text,&text_copy); assert(invalid.code==1); assert(API(result_release)(&invalid)==0);\n"
        " TEXT(Borrowed) preserved=t; invalid=TEXT(view)(NULL,&preserved); assert(invalid.code==1 && preserved.data==t.data && preserved.length==t.length); assert(API(result_release)(&invalid)==0);\n"
        " ok(TEXT(release)(&text)); ok(NUMBER(release)(&number)); ok(CONDITION(release)(&condition)); ok(FAILURE(release)(&failure)); ok(LIST(release)(&list));\n"
        " ok(TEXT(view)(text_copy,&t)); assert(memcmp(t.data,expected,sizeof(expected))==0);\n"
        " ok(NUMBER(view)(number_copy,&n)); assert(memcmp(n.magnitude.data,magnitude,sizeof(magnitude))==0);\n"
        " ok(CONDITION(view)(condition_copy,&c)); assert(c.value==1);\n"
        " ok(FAILURE(view)(failure_copy,&f)); assert(f.failed==1 && memcmp(f.description.data,expected,sizeof(expected))==0);\n"
        " ok(LIST(view)(list_copy,&l)); assert(l.length==2 && memcmp(l.items[1].data,expected,sizeof(expected))==0);\n"
        " ok(TEXT(release)(&text_copy)); ok(NUMBER(release)(&number_copy)); ok(CONDITION(release)(&condition_copy)); ok(FAILURE(release)(&failure_copy)); ok(LIST(release)(&list_copy));\n"
        " assert(text_copy==NULL && number_copy==NULL && condition_copy==NULL && failure_copy==NULL && list_copy==NULL && live_allocations()==0);\n"
        " ok(TEXT(release)(&text_copy)); ok(LIST(release)(&list_copy)); return 0;\n}\n",output);
    int closed=fclose(output); assert(written!=EOF && closed==0);
}
/** Exercise exact public wire views, independent clone lifetime and every partial list failure. */
static void it_owns_independent_wire_snapshots_and_unwinds_every_clone_allocation(void) {
    setup_test(); prepare_project();
    LibraryBuild build; component(&build,"snapshots.component");
    header(&build,"generated/public.h");
    FILE *output=fopen("generated/component.c","wb"); assert(output!=NULL);
    allocator(output);
    bool good=library_abi_write(&build,output); assert(good);
    LibraryHeader writer={.build=&build,.output=output,.prefix=build.identity->internal_name,.good=true};
    library_lifecycle_diagnostics(&writer); assert(writer.good);
    good=library_snapshots_write(&build,output); assert(good);
    fixtures(output,&build);
    int closed=fclose(output); assert(closed==0);
    consumer(&build);
    char errors[32768];
    bool compiled=compile("generated/consumer.c",NULL,"generated/consumer.o",false,NULL,errors,sizeof(errors));
    if(!compiled) { fputs(errors,stderr); } assert(compiled);
    compiled=compile("generated/consumer.c","generated/component.c","generated/consumer",true,"-I" TEST_SDK,errors,sizeof(errors));
    if(!compiled) { fputs(errors,stderr); } assert(compiled);
    execute("generated/consumer");
    library_build_destroy(&build); destroy_test();
}
/** Register independent public compilation and complete partial-prefix owner cleanup. */
void run(void) { it_owns_independent_wire_snapshots_and_unwinds_every_clone_allocation(); }
/** Return only after all counted compiler and generated wire owners were consumed. */
int main(void) { run(); return EXIT_SUCCESS; }
