/** @file
 * @brief Recover saved publication boundaries and preserve modified/conflicting state across independent attempts.
 */
#include "../published/common.c"
#include "tools/relocation/recovery/internal.h"

/** Check both outcomes and a second recovery after legitimate later user edits. */
static void it_recovers_each_decision_without_rewriting_again(void) {
    for (unsigned committed = 0; committed < 2; committed++) {
        prepare(committed ? "committed-replay" : "uncommitted-replay");
        const char *journal = interrupt_publication(committed != 0);
        RelocationRecovery recovery;
        relocation_recovery_init(&recovery,sources.allocator);
        bool result = relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal});
        if (!result) { fprintf(stderr,"Recovery failed: %s\n",recovery.transaction.error.message); }
        assert(result && recovery.ready && recovery.finished && recovery.rolled_back == !committed);
        assert(recovery.transaction.committed == (committed != 0));
        importer(committed ? "@!\nvalue\n.new.a\n" : "@!\nvalue\n.old.a\n");
        struct stat metadata;
        assert(lstat(path(committed ? "project/old" : "project/new"),&metadata) != 0 && errno == ENOENT);
        assert(lstat(path(committed ? "project/new/a" : "project/old/a"),&metadata) == 0);
        relocation_recovery_destroy(&recovery);
        file(committed ? "project/new/a" : "project/old/a","later user bytes\n");
        file("imports/unused","later importer bytes\n");
        relocation_recovery_init(&recovery,sources.allocator);
        assert(relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal}));
        assert(recovery.finished && recovery.rolled_back == !committed);
        importer("later importer bytes\n");
        relocation_recovery_destroy(&recovery);
        complete_fixture();
    }
}
/** A changed published file must retain the original and the modified replacement, then allow retry after exact repair. */
static void it_retains_modified_published_bytes_until_explicit_repair(void) {
    prepare("changed-replay");
    const char *journal = interrupt_publication(false);
    file("project/new/a","user modification\n");
    RelocationRecovery recovery;
    relocation_recovery_init(&recovery,sources.allocator);
    assert(!relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal}));
    assert(recovery.ready && !recovery.finished && recovery.transaction.status == RELOCATION_CHANGED_INPUT);
    const RelocationTransactionAction *source = recovery.transaction.first;
    struct stat metadata;
    assert(lstat(source->slot->backup,&metadata) == 0 && S_ISDIR(metadata.st_mode));
    assert(lstat(path("project/new/a"),&metadata) == 0);
    importer("@!\nvalue\n.new.a\n");
    relocation_recovery_destroy(&recovery);
    file("project/new/a","@!\n");
    relocation_recovery_init(&recovery,sources.allocator);
    assert(relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal}));
    assert(recovery.rolled_back);
    relocation_recovery_destroy(&recovery);
    done();
}
/** Newly created children of a published tree are user data even if every original prepared inode still matches. */
static void it_refuses_unrecorded_tree_children_before_any_inverse(void) {
    prepare("extra-entry-replay");
    const char *journal = interrupt_publication(false);
    file("project/new/extra","user data\n");
    RelocationRecovery recovery;
    relocation_recovery_init(&recovery,sources.allocator);
    assert(!relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal}));
    assert(recovery.transaction.status == RELOCATION_CHANGED_INPUT);
    importer("@!\nvalue\n.new.a\n");
    assert(unlink(path("project/new/extra")) == 0);
    relocation_recovery_destroy(&recovery);
    relocation_recovery_init(&recovery,sources.allocator);
    assert(relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal}));
    relocation_recovery_destroy(&recovery);
    done();
}
/** A committed outcome retires saved originals while preserving legitimate newer public bytes. */
static void it_preserves_later_public_edits_during_committed_cleanup(void) {
    prepare("committed-edits-replay");
    const char *journal = interrupt_publication(true);
    file("project/new/a","after commit\n");
    file("imports/unused","after commit importer\n");
    RelocationRecovery recovery;
    relocation_recovery_init(&recovery,sources.allocator);
    assert(relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal}));
    assert(recovery.transaction.committed && recovery.finished && !recovery.rolled_back);
    importer("after commit importer\n");
    relocation_recovery_destroy(&recovery);
    complete_fixture();
}
/** A pending executor owns the recovery-directory lease; a second executor must refuse before touching its records. */
static void it_refuses_recovery_while_the_writer_holds_exclusive_access(void) {
    prepare("busy-recovery");
    folder("recovery");
    RelocationTransaction transaction;
    relocation_transaction_init(&transaction,sources.allocator);
    transaction.staging = &staging;
    assert(relocation_report_build(&transaction.report,&staging));
    assert(relocation_transaction_lease(&transaction,path("recovery")));
    assert(relocation_transaction_prepare(&transaction));
    assert(relocation_transaction_journal(&transaction));
    RelocationRecovery recovery;
    relocation_recovery_init(&recovery,sources.allocator);
    assert(!relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=transaction.journal}));
    assert(recovery.transaction.status == RELOCATION_BUSY);
    relocation_recovery_destroy(&recovery);
    const char *journal = source_arena_copy(&fixture_paths,transaction.journal);
    relocation_transaction_destroy(&transaction);
    relocation_recovery_init(&recovery,sources.allocator);
    assert(relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal}));
    assert(recovery.rolled_back);
    relocation_recovery_destroy(&recovery);
    done();
}
/** Refuse the identity acknowledgement after mkdir, deliberately leaving a durably reserved but empty private directory. */
static bool deny_acknowledgement(void *context, RelocationIOOperation operation, const char *physical) {
    (void)physical;
    RelocationTransaction *transaction = context;
    return !(operation == RELOCATION_IO_JOURNAL && transaction->slots != transaction->recovery_slot && transaction->slots->created);
}
/** A reserved name alone never authorizes deletion of nonempty data without its acknowledged identity. */
static void it_retains_nonempty_unacknowledged_preparation_reservations(void) {
    prepare("unacknowledged-reservation");
    folder("recovery");
    RelocationTransaction transaction;
    relocation_transaction_init(&transaction,sources.allocator);
    transaction.staging = &staging;
    assert(relocation_report_build(&transaction.report,&staging));
    assert(relocation_transaction_lease(&transaction,path("recovery")));
    const char *target = source_arena_join(&fixture_paths,transaction.recovery_directory,"/","journal");
    transaction.recovery_slot = relocation_transaction_slot(&transaction,target);
    assert(transaction.recovery_slot != NULL);
    transaction.journal = transaction.recovery_slot->directory;
    assert(relocation_transaction_intent(&transaction));
    transaction.io = (RelocationIO){&transaction,deny_acknowledgement};
    assert(relocation_transaction_slot(&transaction,preview.source) == NULL);
    const char *journal = source_arena_copy(&fixture_paths,transaction.journal);
    const char *extra = source_arena_join(&fixture_paths,transaction.slots->directory,"/","user.bin");
    assert(journal != NULL && extra != NULL);
    FILE *stream = fopen(extra,"wb");
    assert(stream != NULL && fputs("user data",stream) >= 0 && fclose(stream) == 0);
    relocation_transaction_destroy(&transaction);
    RelocationRecovery recovery;
    relocation_recovery_init(&recovery,sources.allocator);
    assert(!relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal}));
    if (!recovery.preparing || recovery.transaction.status != RELOCATION_CHANGED_INPUT) {
        fprintf(stderr,"Reservation recovery refused with status=%d: %s\n",recovery.transaction.status,recovery.transaction.error.message);
    }
    assert(recovery.preparing && recovery.transaction.status == RELOCATION_CHANGED_INPUT);
    assert(access(extra,F_OK) == 0);
    untouched(&move_request);
    relocation_recovery_destroy(&recovery);
    assert(unlink(extra) == 0);
    relocation_recovery_init(&recovery,sources.allocator);
    assert(relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal}));
    assert(recovery.preparing && recovery.rolled_back);
    relocation_recovery_destroy(&recovery);
    done();
}
/** Real different devices exercise independent saved-source and prepared-destination ownership through complete recovery. */
static void it_recovers_cross_filesystem_assets_settings_and_invalidations(void) {
    for (unsigned committed = 0; committed < 2; committed++) {
        prepare(committed ? "cross-commit" : "cross-rollback");
        char temporary[] = "/tmp/ama-relocation-recovery-XXXXXX";
        assert(mkdtemp(temporary) != NULL);
        ModuleRoot destination_root = {.path=temporary,.kind=MODULE_ROOT_PROJECT};
        move_request.destination = source_arena_join(&fixture_paths,temporary,"/","new");
        move_request.destination_root = &destination_root;
        const unsigned char opaque[] = {0,255,13,10,0,7};
        bytes("project/old/asset.bin",opaque,sizeof(opaque));
        file("project/old/draft","unfinished draft\n");
        file("project/old/generated.bin","old generated data");
        assert(symlink("cycle.link",path("project/old/cycle.link")) == 0);
        folder("artifacts");
        file("artifacts/cache.bin","outside cache");
        const char *old_configuration = "ama-library | 1\ncomponent | example.component\nabi | ama-c-1\ntarget | test\ncalling-convention | c\nexport | run | .old.a | entry\n";
        const char *new_configuration = "ama-library | 1\ncomponent | example.component\nabi | ama-c-1\ntarget | test\ncalling-convention | c\nexport | run | .new.a | entry\n";
        file("settings.library",old_configuration);
        relocation_staging_destroy(&staging);
        relocation_references_destroy(&references);
        reset_preview();
        assert(relocation_preview_build(&preview,&move_request));
        relocation_references_init(&references,sources.allocator);
        assert(relocation_references_build(&references,&preview));
        relocation_staging_init(&staging,sources.allocator);
        const char *outputs[] = {path("artifacts/cache.bin"),path("project/old/generated.bin")};
        const char *configurations[] = {path("settings.library")};
        assert(relocation_staging_build(&staging,&(RelocationStagingRequest){.references=&references,.entry=".old.a",
            .generated_outputs=outputs,.generated_output_count=2,.library_configurations=configurations,.library_configuration_count=1}));
        struct stat source, destination;
        assert(lstat(preview.source,&source) == 0 && lstat(temporary,&destination) == 0 && source.st_dev != destination.st_dev);
        const char *journal = interrupt_publication(committed != 0);
        RelocationRecovery recovery;
        relocation_recovery_init(&recovery,sources.allocator);
        bool result = relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal});
        if (!result) { fprintf(stderr,"Cross-filesystem recovery failed: %s\n",recovery.transaction.error.message); }
        assert(result && recovery.finished && recovery.rolled_back == !committed);
        assert(strcmp(recovery.transaction.report.entry,".new.a") == 0);
        const char *base = committed ? preview.destination : preview.source;
        const char *asset = source_arena_join(&fixture_paths,base,"/","asset.bin");
        const char *link = source_arena_join(&fixture_paths,base,"/","cycle.link");
        const char *generated = source_arena_join(&fixture_paths,base,"/","generated.bin");
        const SourceText *text;
        assert(source_manager_load(&sources,asset,&text) == SOURCE_OK && source_length(text) == sizeof(opaque)
            && memcmp(source_bytes(text),opaque,sizeof(opaque)) == 0);
        char target[32];
        assert(readlink(link,target,sizeof(target)) == 10 && memcmp(target,"cycle.link",10) == 0);
        const char *configuration = committed ? new_configuration : old_configuration;
        assert(source_manager_load(&sources,path("settings.library"),&text) == SOURCE_OK
            && source_length(text) == strlen(configuration) && memcmp(source_bytes(text),configuration,strlen(configuration)) == 0);
        if (committed) {
            assert(access(generated,F_OK) != 0 && errno == ENOENT);
            assert(access(path("artifacts/cache.bin"),F_OK) != 0 && errno == ENOENT);
            assert(relocation_transaction_remove(&recovery.transaction,preview.destination));
        } else {
            assert(access(generated,F_OK) == 0 && access(path("artifacts/cache.bin"),F_OK) == 0);
        }
        assert(rmdir(temporary) == 0);
        relocation_recovery_destroy(&recovery);
        relocation_recovery_init(&recovery,sources.allocator);
        assert(relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal}));
        assert(recovery.finished && recovery.transaction.committed == (committed != 0));
        relocation_recovery_destroy(&recovery);
        if (committed) { complete_fixture(); } else { done(); }
    }
}
/** Actual persisted inventories are the input to every independent recovery owner. */
void run(void) {
    it_recovers_each_decision_without_rewriting_again();
    it_retains_modified_published_bytes_until_explicit_repair();
    it_refuses_unrecorded_tree_children_before_any_inverse();
    it_preserves_later_public_edits_during_committed_cleanup();
    it_refuses_recovery_while_the_writer_holds_exclusive_access();
    it_retains_nonempty_unacknowledged_preparation_reservations();
    it_recovers_cross_filesystem_assets_settings_and_invalidations();
}
/** The leaf checks production components with assertions enabled. */
int main(void) { run(); return EXIT_SUCCESS; }
