/** @file
 * @brief Emit parent-only allocation counters and C observer declarations for component compositions.
 */
/** Test hooks expose only C controls/metrics; no retained pointer points into an Ama field. */
static void parent_probe(FILE *output) {
    int written=fputs(
        "#include \"parent-api.h\"\n#include \"child-api.h\"\n#include <stdlib.h>\n#include <assert.h>\n"
        "static size_t allocation_calls,live_allocations,fail_at;\n"
        "void select_failure(size_t ordinal) { allocation_calls=0; fail_at=ordinal; }\n"
        "size_t allocations(void) { return allocation_calls; }\nsize_t live(void) { return live_allocations; }\n"
        "static void *parent_malloc(size_t size) { allocation_calls++; if(allocation_calls==fail_at) return NULL; void *p=malloc(size); if(p!=NULL) live_allocations++; return p; }\n"
        "static void parent_free(void *p) { if(p!=NULL) { assert(live_allocations!=0); live_allocations--; free(p); } }\n"
        "extern CHILD(ApiResult) fixture_context_create(CHILD(Context) *);\n"
        "extern CHILD(ApiResult) fixture_instance_create(CHILD(Context),CHILD(Instance) *);\n"
        "extern void fixture_created(CHILD(Context)); extern void fixture_disposed(CHILD(Context));\n"
        "extern void fixture_before_child(CHILD(Context),CHILD(Instance));\n"
        "extern void fixture_after_child(CHILD(Context),CHILD(Instance));\n"
        "#define AMA_EMBED_CONTEXT_CREATE fixture_context_create\n#define AMA_EMBED_CREATE fixture_instance_create\n"
        "#define AMA_EMBED_CREATED fixture_created\n#define AMA_EMBED_DISPOSED fixture_disposed\n"
        "#define AMA_EMBED_BEFORE_CHILD fixture_before_child\n"
        "#define AMA_EMBED_AFTER_CHILD fixture_after_child\n"
        "#define malloc parent_malloc\n#define free parent_free\n",output); assert(written!=EOF);
}
