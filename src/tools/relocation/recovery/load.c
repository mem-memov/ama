/** @file
 * @brief Load a private recovery inventory under descriptor-backed exclusive directory leases.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids concurrent application/recovery and refuses missing or foreign recovery authority.
 */
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "internal.h"
#include "../internal.h"

/** Normalize only existing ordinary directories; the temporary path resolver owner expires after the canonical copy. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool location(RelocationRecovery *recovery, const char *input) {
    RelocationTransaction *transaction = &recovery->transaction;
    RelocationPreview preview;
    relocation_preview_init(&preview,transaction->arena.allocator);
    const char *path = relocation_path(&preview,input,false);
    RelocationStatus status = preview.status;
    if (path != NULL) {
        transaction->journal = source_arena_copy(&transaction->arena,path);
        if (transaction->journal == NULL) { status = RELOCATION_OUT_OF_MEMORY; }
    }
    relocation_preview_destroy(&preview);
    return transaction->journal != NULL
        || relocation_transaction_fail(transaction,status,"The recovery directory could not be resolved exactly",NULL);
}
/** Each private regular record is stable under the journal directory lease; reading it does not grant mutation authority. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool read_record(RelocationRecovery *recovery, const char *path, const SourceText **output) {
    RelocationTransaction *transaction = &recovery->transaction;
    struct stat before, after;
    if (lstat(path,&before) != 0 || !S_ISREG(before.st_mode)
        || before.st_uid != geteuid() || before.st_nlink != 1 || (before.st_mode&0077) != 0) {
        return relocation_transaction_fail(transaction,RELOCATION_INVALID_JOURNAL,"A recovery record is missing or lacks private regular-file ownership",path);
    }
    if (!relocation_transaction_permit(transaction,RELOCATION_IO_READ,path)) { return false; }
    SourceStatus status = source_manager_load(&recovery->sources,path,output);
    if (status != SOURCE_OK) {
        return relocation_transaction_fail(transaction,status == SOURCE_OUT_OF_MEMORY ? RELOCATION_OUT_OF_MEMORY : RELOCATION_IO_ERROR,
            "A recovery record could not be read",path);
    }
    if (lstat(path,&after) != 0 || before.st_dev != after.st_dev || before.st_ino != after.st_ino
        || before.st_size != after.st_size || before.st_mtim.tv_sec != after.st_mtim.tv_sec || before.st_mtim.tv_nsec != after.st_mtim.tv_nsec
        || before.st_ctim.tv_sec != after.st_ctim.tv_sec || before.st_ctim.tv_nsec != after.st_ctim.tv_nsec) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A recovery record changed while being read",path);
    }
    return true;
}
/** A prepared inventory must describe precisely the same owned scope and private directories as its earlier preparation authority. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool prepared(RelocationRecovery *recovery) {
    RelocationTransaction *transaction = &recovery->transaction;
    RelocationReport intent_report = transaction->report;
    RelocationTransactionSlot *intent_slots = transaction->slots;
    transaction->slots = NULL;
    transaction->recovery_slot = NULL;
    transaction->report.modules = NULL;
    transaction->report.files = NULL;
    if (!read_record(recovery,transaction->journal_record,&recovery->record)
        || !relocation_journal_decode(recovery,recovery->record) || !relocation_journal_validate(recovery)
        || !relocation_recovery_same_intent(recovery,&intent_report,intent_slots)) { return false; }
    transaction->retain_receipt = true;
    recovery->preparing = false;
    return true;
}
/** Preparation authority precedes plan authority; READY is durable before any public source path can change. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_recovery_load(RelocationRecovery *recovery, const char *input) {
    RelocationTransaction *transaction = &recovery->transaction;
    if (!location(recovery,input) || !relocation_transaction_lease_parent(transaction,transaction->journal)
        || !relocation_transaction_lease_directory(transaction,transaction->journal)) { return false; }
    struct stat directory;
    if (lstat(transaction->journal,&directory) != 0 || directory.st_uid != geteuid() || (directory.st_mode&0077) != 0) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"The recovery directory lacks private current-user ownership",transaction->journal);
    }
    transaction->journal_record = source_arena_join(&transaction->arena,transaction->journal,"/","plan");
    transaction->intent_record = source_arena_join(&transaction->arena,transaction->journal,"/","intent");
    if (transaction->journal_record == NULL || transaction->intent_record == NULL) {
        return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Recovery record path allocation failed",transaction->journal);
    }
    bool ready;
    if (!read_record(recovery,transaction->intent_record,&recovery->record)
        || !relocation_intent_decode(recovery,recovery->record,&ready)) { return false; }
    const RelocationTransactionSlot *owned = transaction->recovery_slot;
    if (strcmp(transaction->journal,owned->directory) != 0 || directory.st_dev != owned->metadata.st_dev
        || directory.st_ino != owned->metadata.st_ino) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"The journal belongs to a different recovery directory identity",transaction->journal);
    }
    if (ready && !prepared(recovery)) { return false; }
    transaction->report.ready = true;
    recovery->ready = true;
    return true;
}
