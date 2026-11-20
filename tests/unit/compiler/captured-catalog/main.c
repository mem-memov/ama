/** @file
 * @brief Verify source-only compiler snapshots share actual static semantics without inventing entries or input-cache authority.
 * @see [T193 contract](../../../../docs/implementation/static-relocation.md)
 */
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include "../../source/common.c"
#include "compiler/frontend.h"

/** Captured bytes have projected source locations but never require those paths to exist. */
static const SourceText *text(const char *path, const char *bytes) {
    const SourceText *result = NULL;
    assert(source_manager_add(&sources,path,(const unsigned char *)bytes,strlen(bytes),&result) == SOURCE_OK);
    return result;
}
/** Shared semantic admission consumes complete source/ABI authority while native C/header paths remain opaque. */
static void it_checks_closed_static_exports_and_captured_native_future_paths_without_an_entry(void) {
    setup_test();
    CompilerFrontend frontend;
    compiler_frontend_init(&frontend,sources.allocator);
    ModuleSnapshotEntry entries[] = {
        {".value",text("generated/not-created/value","@C\n"),1,1},
        {".box",text("generated/not-created/box","@<\nvalue\n...slot\n@!\n"),1,2},
        {".factory",text("generated/not-created/factory","@...\n...out\n.box\n ...slot < .value\n"),1,3},
        {".main",text("generated/not-created/main","@!\nvalue\n.factory\n ...out\n"),1,4}
    };
    assert(module_catalog_snapshot(&frontend.project.catalog,entries,4) == MODULE_OK);
    const ModuleSource *native = NULL;
    assert(module_catalog_find(&frontend.project.catalog,".value",&native) == MODULE_OK);
    const SourceText *abi = text("generated/not-created/value.abi","+ initialize\n- destroy\n= clone\n");
    assert(module_catalog_snapshot_native(&frontend.project.catalog,native,
        "generated/not-created/value.c","generated/not-created/value.h",abi) == MODULE_OK);
    const char *required[] = {".value",".box",".factory",".main"};
    bool valid = compiler_frontend_check_snapshot(&frontend,required,4);
    if (!valid) { fprintf(stderr,"Captured compiler: %s\n",frontend.error.message); }
    assert(valid && !frontend.valid && frontend.project.entry == NULL);
    /* Composite readiness is reserved for executable planning; source metadata
     * has its own completed phase certificates. */
    assert(frontend.specializations.valid && frontend.semantic.relations_valid && frontend.semantic.constants_valid);
    assert(semantic_find_type(&frontend.semantic,".box") && !semantic_find_type(&frontend.semantic,".factory"));
    assert(!frontend.inputs.attempted && !frontend.inputs.sealed && !frontend.inputs.compared && !frontend.inputs.first);
    assert(!frontend.plan_initialized);
    struct stat metadata;
    assert(lstat("generated/not-created/value.c",&metadata) == -1 && errno == ENOENT);
    compiler_frontend_destroy(&frontend);
    destroy_test();
}
/** Empty domains are checked directly; source-only open templates acquire no implicit concrete Type. */
static void it_checks_empty_and_open_definition_only_catalogs_without_default_instances(void) {
    for (size_t open = 0; open < 2; open++) {
        setup_test();
        CompilerFrontend frontend;
        compiler_frontend_init(&frontend,sources.allocator);
        ModuleSnapshotEntry entry = {".open",text("generated/not-created/open","@<\nvalue\n...slot\n@!\n"),1,1};
        assert(module_catalog_snapshot(&frontend.project.catalog,open ? &entry : NULL,open) == MODULE_OK);
        const char *required[] = {".open"};
        assert(compiler_frontend_check_snapshot(&frontend,open ? required : NULL,open));
        assert(!frontend.valid && frontend.project.entry == NULL && frontend.semantic.first == NULL);
        assert(!frontend.plan_initialized && !frontend.inputs.sealed);
        compiler_frontend_destroy(&frontend);
        destroy_test();
    }
}
/** Run independent captured-owner and empty/source-only boundary observations. */
static void run(void) {
    it_checks_closed_static_exports_and_captured_native_future_paths_without_an_entry();
    it_checks_empty_and_open_definition_only_catalogs_without_default_instances();
}
/** Both original captured bytes and all compiler borrowers retire under counted allocation ownership. */
int main(void) { run(); return 0; }
