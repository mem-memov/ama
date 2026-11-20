/** @file
 * @brief Run independently compiled template programs before and after real directory publication.
 * @see [T193 contract](../../../../docs/implementation/static-relocation.md)
 */
#include "../execution/common.c"
#include "tools/relocation/transaction/transaction.h"
#include "native/namespace/namespace.h"

static const char before_source[] =
    "@!\nvalue\n.value\n |7\nnumber\n..integer\n |23\n"
    "first\n.old.factory\n ...old.value\nsecond\n.old.factory\n ...old.integer\n"
    "first !\n < value\n > copy\nsecond !\n < number\n > integer copy\n"
    "worker\n.worker\nworker !\n < copy\nshow\n.showindex\nshow !\n < integer copy\n";
static const char after_source[] =
    "@!\nvalue\n.value\n |7\nnumber\n..integer\n |23\n"
    "first\n.renamed.factory\n ...old.value\nsecond\n.renamed.factory\n ...old.integer\n"
    "first !\n < value\n > copy\nsecond !\n < number\n > integer copy\n"
    "worker\n.worker\nworker !\n < copy\nshow\n.showindex\nshow !\n < integer copy\n";

/** Read the actual arbitrary-precision Standard integer through its declared index protocol, without assuming payload layout. */
static bool observe_integer(FILE *body, const ScaffoldType *type) {
    if (type->plan->type->module != project_find(&frontend.project,".showindex")) { return false; }
    const SemanticSymbol *input = semantic_interface_position(type->plan->type,SYMBOL_PARAMETER,0);
    assert(input);
    const ScaffoldType *provider = assembly.types;
    while (provider && provider->plan->type != input->type) { provider = provider->next; }
    assert(provider && provider->plan->type->abi.bindings[ABI_INDEX].present);
    assert(fputs("size_t value=0; AmaStatus converted=",body) != EOF);
    assert(native_namespace_symbol(body,provider->native_prefix,provider->plan->type->abi.bindings[ABI_INDEX].function));
    assert(fputs("(parameters->AMA_INPUT(0)->payload,&value); if(converted.code!=AMA_OK) { *status=converted; return; } printf(\"value=%zu\\n\",value);\n",body) != EOF);
    return true;
}

/** Both source specializations copy actual values; concrete native consumers independently observe their results. */
static void fixture(void) {
    directory("generated/project");
    directory("generated/project/old");
    file("generated/project/old/box","@<\ninput\n...old.slot\n@>\noutput\n...old.slot\n input\n@!\noutput\n input\n");
    file("generated/project/old/factory","@...\n...old.value\n.old.box\n ...old.slot < .value\n...old.integer\n.old.box\n ...old.slot < ..integer\n");
    native_entry_override = observe_integer;
    native_fixture("showindex","@<\ninput\n..integer\n@C\n");
}

/** Complete relocation owns independent checked snapshots and retires them after actual publication. */
static void move(const char *source, const char *destination, const char *recovery) {
    setup_test();
    ModuleRoot roots[] = {{"generated/project",NULL,0,MODULE_ROOT_PROJECT},
        {TEST_STDLIB,NULL,0,TEST_STDLIB_KIND}};
    RelocationRequest request = {roots,2,source,destination,NULL};
    RelocationPreview preview;
    relocation_preview_init(&preview,sources.allocator);
    assert(relocation_preview_build(&preview,&request));
    RelocationReferences references;
    relocation_references_init(&references,sources.allocator);
    assert(relocation_references_build(&references,&preview));
    RelocationStaging staging;
    relocation_staging_init(&staging,sources.allocator);
    const char *outputs[] = {"generated/output"};
    assert(relocation_staging_build(&staging,&(RelocationStagingRequest){.references=&references,.entry=".main",
        .generated_outputs=outputs,.generated_output_count=1}));
    directory(recovery);
    RelocationTransaction transaction;
    relocation_transaction_init(&transaction,sources.allocator);
    bool applied = relocation_transaction_apply(&transaction,&(RelocationTransactionRequest){.staging=&staging,.recovery_directory=recovery});
    if (!applied) { fprintf(stderr,"Compiled relocation: %s\n",transaction.error.message); }
    assert(applied && transaction.committed);
    struct stat metadata;
    assert(lstat(source,&metadata) == -1 && errno == ENOENT);
    assert(lstat(destination,&metadata) == 0 && S_ISDIR(metadata.st_mode));
    assert(lstat("generated/output",&metadata) == -1 && errno == ENOENT);
    relocation_transaction_destroy(&transaction);
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    relocation_preview_destroy(&preview);
    destroy_test();
}

/** Constructor origins change logically; graph aliases remain atomic, and actual generated values remain identical. */
static void it_compiles_and_executes_both_closed_exports_after_logical_template_relocation(void) {
    fixture();
    fprintf(stderr,"T193 execution: original template project\n");
    execute_project(before_source,"7:1:0\nvalue=23\n",true);
    move("generated/project/old","generated/project/renamed","generated/recovery-logical");
    fprintf(stderr,"T193 execution: logically moved templates\n");
    execute_project(after_source,"7:1:0\nvalue=23\n",true);
    file("generated/project/renamed/box","@<\ninput\n...old.slot\n |bad\n@>\noutput\n...old.slot\n input\n@!\noutput\n input\n");
    fprintf(stderr,"T193 execution: relocated construction failure\n");
    execute_project(after_source,"",false);
    expect_diagnostic("Not a decimal value");
    expect_diagnostic("generated/project/renamed/box:");
    assert(rename("generated/project","generated/logical-finished") == 0);
}

/** A physical whole-root move retains logical native identities and opaque body bytes across an independent real rebuild. */
static void it_rebuilds_a_whole_moved_root_without_rewriting_native_companion_bytes(void) {
    fixture();
    execute_project(before_source,"7:1:0\nvalue=23\n",true);
    fprintf(stderr,"T193 execution: physical root prepared\n");
    SourceManager original;
    source_manager_init(&original,source_system_allocator());
    const char *names[] = {"value.c","worker.c","showindex.c"};
    const SourceText *bodies[3];
    for (size_t index = 0; index < 3; index++) {
        char path[128];
        int count = snprintf(path,sizeof(path),"generated/project/%s",names[index]);
        assert(count > 0 && (size_t)count < sizeof(path));
        assert(source_manager_load(&original,path,&bodies[index]) == SOURCE_OK);
    }
    move("generated/project","generated/moved-project","generated/recovery-physical");
    ModuleRoot final_roots[] = {{"generated/moved-project",NULL,0,MODULE_ROOT_PROJECT},
        {TEST_STDLIB,NULL,0,TEST_STDLIB_KIND}};
    fprintf(stderr,"T193 execution: physically moved root\n");
    project_roots_override = final_roots;
    project_roots_override_count = 2;
    execute_project(before_source,"7:1:0\nvalue=23\n",true);
    project_roots_override = NULL;
    project_roots_override_count = 0;
    for (size_t index = 0; index < 3; index++) {
        char path[128];
        int count = snprintf(path,sizeof(path),"generated/moved-project/%s",names[index]);
        assert(count > 0 && (size_t)count < sizeof(path));
        const SourceText *after = NULL;
        assert(source_manager_load(&original,path,&after) == SOURCE_OK);
        assert(source_length(after) == source_length(bodies[index]));
        assert(memcmp(source_bytes(after),source_bytes(bodies[index]),source_length(after)) == 0);
    }
    source_manager_destroy(&original);
}

/** Compare independently specified runtime output and real file state, not prepared relocation buffers. */
static void run(void) {
    it_compiles_and_executes_both_closed_exports_after_logical_template_relocation();
    it_rebuilds_a_whole_moved_root_without_rewriting_native_companion_bytes();
}
/** Every generated execution and relocation owner retires before success. */
int main(void) { run(); native_entry_override = NULL; return 0; }
