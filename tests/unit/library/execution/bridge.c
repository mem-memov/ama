/** @file
 * @brief Generate real public exports and count native/runtime/wire resource ownership.
 */
/** Make every native and wire allocation visible without retaining any Ama reference. */
static void allocator(FILE *output) {
    int written=fputs(
        "#include \"public.h\"\n#include <stdlib.h>\n#include <assert.h>\n"
        "static size_t calls,fail,live;\nvoid select_failure(size_t offset) { calls=0; fail=offset; }\n"
        "size_t allocation_calls(void) { return calls; }\nsize_t live_allocations(void) { return live; }\n"
        "static void *execution_malloc(size_t size) { calls++; if(calls==fail) return NULL; void *p=malloc(size); if(p!=NULL) live++; return p; }\n"
        "static void execution_free(void *p) { if(p!=NULL) { assert(live!=0); live--; free(p); } }\n"
        "#define malloc execution_malloc\n#define free execution_free\n#include \"output/project.c\"\n",output);
    assert(written!=EOF);
}
/** Emit aliases for the separately selected native root and read-only alias of the Ama root. */
static void export_aliases(FILE *output, const LibraryBuild *build) {
    const LibraryExport *counter=build->exports->next, *observer=counter->next;
    assert(counter!=NULL && observer!=NULL);
    int written=fprintf(output,"#define COUNTER(name) %s_ ## name\n#define OBSERVER(name) %s_ ## name\n",
        counter->identity->internal_name,observer->identity->internal_name);
    assert(written>0);
}
/** Generate actual checked implementations, not a substitute C execution engine. */
static void bridge(const LibraryBuild *build, NativeScaffold *assembly) {
    FILE *output=fopen("generated/component.c","wb"); assert(output!=NULL); allocator(output);
    bool written=library_abi_write(build,output); assert(written);
    uint64_t types[3]; size_t count=0;
    for (const LibraryExport *exported=build->exports; exported!=NULL; exported=exported->next) {
        assert(count<3); types[count++]=codegen_type_id(assembly,native_scaffold_type(assembly,exported->type));
    }
    written=library_lifecycle_write(build,types,count,output); assert(written);
    written=library_snapshots_write(build,output); assert(written);
    written=library_codecs_write(build,assembly,output); assert(written);
    int closed=fclose(output); assert(closed==0);
}
/** Public-only declarations also expose the test allocator's scalar observations. */
static void consumer_header(const LibraryBuild *build) {
    FILE *output=fopen("generated/consumer-api.h","wb"); assert(output!=NULL);
    int written=fputs("#include \"public.h\"\n",output); assert(written!=EOF);
    aliases(output,build); export_aliases(output,build);
    written=fputs("void select_failure(size_t); size_t allocation_calls(void); size_t live_allocations(void);\n",output);
    int closed=fclose(output); assert(written!=EOF && closed==0);
}
