/** @file
 * @brief Verify source-only compiler snapshots share actual static semantics without inventing entries or input-cache authority.
 * @see [T193 contract](../../../../docs/implementation/static-relocation.md)
 */
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include "../../source/common.c"
#include "compiler/frontend.h"
#include "delivery/source/entry.h"

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
        {"/test/fixture/v1.0.0/src/value",text("generated/not-created/value","@C\n"),1,1, NULL},
        {"/test/fixture/v1.0.0/src/box",text("generated/not-created/box","@<\nvalue\n.slot\n@!\n"),1,2, NULL},
        {"/test/fixture/v1.0.0/src/factory",text("generated/not-created/factory","@...\n.out\n/test/fixture/v1.0.0/src/box\n .slot < /test/fixture/v1.0.0/src/value\n"),1,3, NULL},
        {"/test/fixture/v1.0.0/src/main",text("generated/not-created/main","@!\nvalue\n/test/fixture/v1.0.0/src/factory\n .out\n"),1,4, NULL}
    };
    assert(module_catalog_snapshot(&frontend.project.catalog,entries,4) == MODULE_OK);
    assert(module_catalog_capture_version(&frontend.project.catalog,"/test/fixture/v1.0.0",
        "generated/not-created",text("generated/not-created/package.ini","")) == MODULE_OK);
    const ModuleSource *native = NULL;
    assert(module_catalog_find(&frontend.project.catalog,"/test/fixture/v1.0.0/src/value",&native) == MODULE_OK);
    const SourceText *abi = text("generated/not-created/value.abi","+ initialize\n- destroy\n= clone\n");
    assert(module_catalog_snapshot_native(&frontend.project.catalog,native,
        "generated/not-created/value.c","generated/not-created/value.h",abi) == MODULE_OK);
    const char *required[] = {"/test/fixture/v1.0.0/src/value","/test/fixture/v1.0.0/src/box","/test/fixture/v1.0.0/src/factory","/test/fixture/v1.0.0/src/main"};
    bool valid = compiler_frontend_check_snapshot(&frontend,required,4);
    if (!valid) { fprintf(stderr,"Captured compiler: %s\n",frontend.error.message); }
    assert(valid && !frontend.valid && frontend.project.entry == NULL);
    /* Composite readiness is reserved for executable planning; source metadata
     * has its own completed phase certificates. */
    assert(frontend.specializations.valid && frontend.semantic.relations_valid && frontend.semantic.constants_valid);
    assert(semantic_find_type(&frontend.semantic,"/test/fixture/v1.0.0/src/box") && !semantic_find_type(&frontend.semantic,"/test/fixture/v1.0.0/src/factory"));
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
        ModuleSnapshotEntry entry = {"/test/fixture/v1.0.0/src/open",text("generated/not-created/open","@<\nvalue\n.slot\n@!\n"),1,1, NULL};
        assert(module_catalog_snapshot(&frontend.project.catalog,open ? &entry : NULL,open) == MODULE_OK);
        if (open) {
            assert(module_catalog_capture_version(&frontend.project.catalog,"/test/fixture/v1.0.0",
                "generated/not-created",text("generated/not-created/package.ini","")) == MODULE_OK);
        }
        const char *required[] = {"/test/fixture/v1.0.0/src/open"};
        assert(compiler_frontend_check_snapshot(&frontend,open ? required : NULL,open));
        assert(!frontend.valid && frontend.project.entry == NULL && frontend.semantic.first == NULL);
        assert(!frontend.plan_initialized && !frontend.inputs.sealed);
        compiler_frontend_destroy(&frontend);
        destroy_test();
    }
}
/** Captured package/local environments resolve raw aliases without consulting future files. */
static void it_checks_captured_scopes_and_whole_constant_aliases_without_filesystem_fallback(void) {
    for (size_t local = 0; local < 2; local++) {
        setup_test();
        CompilerFrontend frontend;
        compiler_frontend_init(&frontend, sources.allocator);
        const SourceText *configuration = text("generated/not-created/scoped/src/main.ini",
            "~body = ~types/value\n~constant = ~types/palette/shared\n");
        ModuleSnapshotEntry entries[] = {
            {"/test/fixture/v1.0.0/src/main", text("generated/not-created/scoped/src/main",
                "@!\nstate\n~body\nvalue\n ~constant\nadapter\n//adapter\nadapter !\n < state\n"),
                1, 101, local ? configuration : NULL},
            {"/test/fixture/v1.0.0/src/value", text("generated/not-created/scoped/src/value", "@!\n"), 1, 102, NULL},
            {"/test/fixture/v1.0.0/src/palette", text("generated/not-created/scoped/src/palette",
                "@=\nshared\n~types/value\n@!\n"), 1, 103, NULL},
            {"//adapter", text("generated/not-created/standard/adapter",
                "@<\nargument\n.slot\n@!\n"), 1, 104, NULL}
        };
        assert(module_catalog_snapshot(&frontend.project.catalog, entries, 4) == MODULE_OK);
        assert(module_catalog_capture_version(&frontend.project.catalog, "/test/fixture/v1.0.0",
            "generated/not-created/scoped", text("generated/not-created/scoped/package.ini",
                "[aliases]\n~types = ./src\n")) == MODULE_OK);
        const char *required[] = {"/test/fixture/v1.0.0/src/main"};
        bool valid = compiler_frontend_check_snapshot(&frontend, required, 1);
        assert(valid == (local != 0));
        if (local) {
            const ProjectModule *main = project_find(&frontend.project, required[0]);
            assert(main && main->aliases->count == 2 && main->aliases->package->count == 1);
            assert(main->first->target == project_find(&frontend.project, "/test/fixture/v1.0.0/src/value"));
            assert(main->first->next->constant_path
                && strcmp(main->first->next->constant_path->module->text,
                    "/test/fixture/v1.0.0/src/palette") == 0);
            const ProjectModule *adapter = project_find(&frontend.project, "//adapter");
            assert(adapter && adapter->first == NULL && adapter->aliases->package == NULL);
        } else {
            assert(frontend.project.status == PROJECT_INVALID_CONFIGURATION);
            assert(diagnostic_valid(&frontend.error));
        }
        struct stat metadata;
        int found = lstat("generated/not-created/scoped/package.ini", &metadata);
        assert(found == -1 && errno == ENOENT);
        compiler_frontend_destroy(&frontend);
        destroy_test();
    }
}

/** Prepare exact future-path sources with native bytes already captured. */
static void prepare_entry(CompilerFrontend *frontend) {
    compiler_frontend_init(frontend, sources.allocator);
    static const char *const identities[] = {"//value", "//palette", "//grounded", "//open", "//main", "//bad", "//unknown", "//producer"};
    static const char *const paths[] = {"generated/not-created/value", "generated/not-created/palette", "generated/not-created/grounded",
        "generated/not-created/open", "generated/not-created/main", "generated/not-created/bad", "generated/not-created/unknown", "generated/not-created/producer"};
    static const char *const code[] = {"@C\n", "@=\nvalue\n//value\n", "@>\nresult\n.T\n //palette/value\n@!\n",
        "@<\nvalue\n.T\n@!\n", "@!\nnode\n//value\n", "@!\nmissing !\n",
        "@!\nworker\n//producer\nworker !\n > fresh\n", "@>\nresult\n.T\n@!\n"};
    ModuleSnapshotEntry entries[8];
    for (size_t index = 0; index < 8; index++) {
        entries[index] = (ModuleSnapshotEntry){identities[index], text(paths[index], code[index]), 1, (ino_t)index + 1, NULL};
    }
    assert(module_catalog_snapshot(&frontend->project.catalog, entries, 8) == MODULE_OK);
    const ModuleSource *native = NULL;
    assert(module_catalog_find(&frontend->project.catalog, "//value", &native) == MODULE_OK);
    assert(module_catalog_snapshot_native(&frontend->project.catalog, native, "generated/not-created/value.c",
        "generated/not-created/value.h", text("generated/not-created/value.abi", "+ initialize\n- destroy\n= clone\n")) == MODULE_OK);
}
/** A real complete plan proves eligibility without any sealed filesystem certificate. */
static void it_checks_captured_executable_and_grounded_entries_without_physical_input_capture(void) {
    const char *entries[] = {"//main", "//grounded"};
    for (size_t index = 0; index < 2; index++) {
        setup_test(); CompilerFrontend frontend; prepare_entry(&frontend);
        assert(compiler_frontend_check_entry_snapshot(&frontend, entries[index]));
        assert(frontend.plan_initialized && frontend.plan.valid && !frontend.valid);
        assert(delivery_source_entry_state(&frontend) == DELIVERY_ENTRY_EXECUTABLE);
        assert(!frontend.inputs.attempted && !frontend.inputs.sealed && !frontend.inputs.first && !frontend.inputs.compared);
        if (index) {
            const StaticModule *module = static_module_find(&frontend.static_inventory, frontend.project.entry);
            assert(module && module->parameter_count == 1);
        }
        struct stat metadata; assert(lstat("generated/not-created/value.c", &metadata) == -1 && errno == ENOENT);
        assert(!compiler_frontend_check_entry_snapshot(&frontend, entries[index]) && frontend.plan.valid);
        compiler_frontend_destroy(&frontend); destroy_test();
    }
}
/** Only a pending original root key is Open; unrelated unknown outputs and body errors remain Invalid. */
static void it_preserves_exact_open_root_and_invalid_captured_entry_classification(void) {
    const char *entries[] = {"//open", "//bad", "//unknown", "//palette"};
    for (size_t index = 0; index < 4; index++) {
        setup_test(); CompilerFrontend frontend; prepare_entry(&frontend);
        assert(!compiler_frontend_check_entry_snapshot(&frontend, entries[index]) && !frontend.valid);
        assert(delivery_source_entry_state(&frontend) == (index ? DELIVERY_ENTRY_INVALID : DELIVERY_ENTRY_OPEN));
        assert(frontend.error.message && !frontend.inputs.attempted && !frontend.inputs.sealed);
        compiler_frontend_destroy(&frontend); destroy_test();
    }
}
/** Each attempted proof allocation either completes or retains a destructible unsealed failure. */
static void it_retires_every_captured_entry_proof_allocation_failure(void) {
    setup_test(); CompilerFrontend frontend; prepare_entry(&frontend); size_t before = allocation_count;
    assert(compiler_frontend_check_entry_snapshot(&frontend, "//main"));
    size_t count = allocation_count - before; compiler_frontend_destroy(&frontend); destroy_test(); assert(count);
    for (size_t ordinal = 1; ordinal <= count; ordinal++) {
        setup_test(); prepare_entry(&frontend); failure_at = allocation_count + ordinal;
        assert(!compiler_frontend_check_entry_snapshot(&frontend, "//main") && !frontend.valid);
        assert(delivery_source_entry_state(&frontend) == DELIVERY_ENTRY_INVALID && !frontend.inputs.sealed);
        failure_at = 0; compiler_frontend_destroy(&frontend); destroy_test();
    }
}

/** Run independent captured-owner and empty/source-only boundary observations. */
static void run(void) {
    it_checks_captured_scopes_and_whole_constant_aliases_without_filesystem_fallback();
    it_checks_closed_static_exports_and_captured_native_future_paths_without_an_entry();
    it_checks_empty_and_open_definition_only_catalogs_without_default_instances();
    it_checks_captured_executable_and_grounded_entries_without_physical_input_capture();
    it_preserves_exact_open_root_and_invalid_captured_entry_classification();
    it_retires_every_captured_entry_proof_allocation_failure();
}
/** Both original captured bytes and all compiler borrowers retire under counted allocation ownership. */
int main(void) { run(); return 0; }
