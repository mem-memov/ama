/**
 * @file
 * @brief Verify cross-device ordinary package placement and untouched destination metadata.
 */
#include "../preflight/common.c"
#include "../../../../fixtures/package.h"
#include "compiler/frontend.h"

/** Refuse the importer's publication once, after the selected tree is published.
 * A named operation/path selects this boundary; no guessed checkpoint ordinal
 * or timing assumption controls rollback.
 */
typedef struct { const char *path; bool refused; } ImporterRefusal;

/** Permit inverse/cleanup operations after the one explicit importer refusal. */
static bool refuse_importer(void *context, RelocationIOOperation operation, const char *path) {
    ImporterRefusal *refusal = context;
    if (!refusal->refused && operation == RELOCATION_IO_PUBLISH
        && strcmp(path, refusal->path) == 0) {
        refusal->refused = true;
        return false;
    }
    return true;
}

/** Rebuild the real immutable plan after configuring a different ordinary package
 * on another actual filesystem. Its pre-existing configuration is outside S/D.
 */
static void prepare_placement(const char *name, const char *destination_root) {
    prepare(name);
    fixture_package_prepare(destination_root, "destination");
    ModuleRoot added = {.path = destination_root, .kind = MODULE_ROOT_PROJECT};
    move_request.destination = source_arena_join(&fixture_paths, destination_root,
        "/packages/test/destination/v1.0.0/src/", "new");
    assert(move_request.destination != NULL);
    move_request.destination_root = &added;
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    reset_preview();
    assert(relocation_preview_build(&preview, &move_request));
    /* The preview owns copied roots; the request's stack descriptor is no longer used. */
    move_request.destination_root = NULL;
    relocation_references_init(&references, sources.allocator);
    assert(relocation_references_build(&references, &preview));
    relocation_staging_init(&staging, sources.allocator);
    assert(relocation_staging_build(&staging, &(RelocationStagingRequest){.references = &references}));
    struct stat source, destination;
    assert(lstat(preview.source, &source) == 0 && lstat(destination_root, &destination) == 0);
    assert(source.st_dev != destination.st_dev);
}

/** Independent source reads distinguish the committed and restored importer. */
static void expect_importer(bool committed) {
    const char *expected = committed
        ? "@!\nvalue\n/test/destination/v1.0.0/src/new/a\n"
        : "@!\nvalue\n/test/fixture/v1.0.0/src/old/a\n";
    const SourceText *source;
    assert(source_manager_load(&sources, path("imports/packages/test/imports/v1.0.0/src/unused"), &source) == SOURCE_OK);
    assert(source_length(source) == strlen(expected));
    assert(memcmp(source_bytes(source), expected, strlen(expected)) == 0);
}

/** Commit and explicit mid-publication rollback preserve a destination package's
 * configuration inode/bytes, and the actual compiler accepts the resulting roots.
 */
static void it_places_ordinary_packages_across_devices_and_restores_failed_publication(void) {
    for (size_t refused = 0; refused < 2; refused++) {
        char temporary[] = "/tmp/ama-package-placement-XXXXXX";
        assert(mkdtemp(temporary) != NULL);
        prepare_placement(refused ? "package-rollback" : "package-commit", temporary);
        const char *configuration = source_arena_join(&fixture_paths, temporary,
            "/packages/test/destination/v1.0.0/", "package.ini");
        assert(configuration != NULL);
        struct stat before, after;
        assert(lstat(configuration, &before) == 0 && S_ISREG(before.st_mode) && before.st_size == 0);
        const RelocationModule *importer = module("/test/imports/v1.0.0/src/unused");
        assert(importer != NULL);
        ImporterRefusal refusal = {.path = importer->before_path};
        RelocationTransaction transaction;
        application(&transaction);
        RelocationTransactionRequest request = {.staging = &staging, .recovery_directory = path("recovery1")};
        if (refused) { request.io = (RelocationIO){.context = &refusal, .permit = refuse_importer}; }
        size_t open_descriptors = descriptors();
        bool applied = relocation_transaction_apply(&transaction, &request);
        assert(descriptors() == open_descriptors);
        assert(applied == !refused && transaction.committed == !refused);
        assert(refusal.refused == (refused != 0));
        if (refused) { assert(transaction.state == RELOCATION_TRANSACTION_ROLLED_BACK); }
        assert(lstat(configuration, &after) == 0 && before.st_dev == after.st_dev
            && before.st_ino == after.st_ino && before.st_size == after.st_size);
        expect_importer(!refused);
        CompilerFrontend consumer;
        compiler_frontend_init(&consumer, sources.allocator);
        assert(compiler_frontend_check(&consumer,
            refused ? preview.before_roots : preview.after_roots,
            refused ? preview.before_root_count : preview.after_root_count,
            "/test/imports/v1.0.0/src/unused"));
        compiler_frontend_destroy(&consumer);
        assert(relocation_transaction_remove(&transaction, temporary));
        relocation_transaction_destroy(&transaction);
        if (refused) { done(); }
        else {
            relocation_staging_destroy(&staging);
            relocation_references_destroy(&references);
            reset_preview();
            finish();
        }
    }
}

/** Register the exact placement and rollback case in the independent leaf. */
void run(void) {
    it_places_ordinary_packages_across_devices_and_restores_failed_publication();
}

/** Execute actual filesystem outcomes, not merely a prepared in-memory model. */
int main(void) { run(); return 0; }
