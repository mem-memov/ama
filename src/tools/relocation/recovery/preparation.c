/** @file
 * @brief Retire explicitly registered interrupted preparation copies without touching source paths.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * never treats an unacknowledged nonempty directory as owned recovery data.
 */
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "internal.h"
#include "../internal.h"

/** A recorded reservation without an identity can own only an absent or still-empty private directory. */
static bool contents(RelocationTransaction *transaction, RelocationTransactionSlot *slot, bool empty) {
    DIR *directory = opendir(slot->directory);
    if (directory == NULL) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"An unacknowledged preparation directory could not be inspected",slot->directory); }
    bool complete = true;
    for (;;) {
        errno = 0;
        struct dirent *entry = readdir(directory);
        if (entry == NULL) {
            if (errno != 0) { complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Preparation reservation enumeration failed",slot->directory); }
            break;
        }
        if (strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0
            && (empty || strcmp(entry->d_name,"prepared") != 0)) {
            complete = relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A preparation reservation contains unrecorded data",slot->directory);
            break;
        }
    }
    if (closedir(directory) != 0) { complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Preparation reservation enumeration could not be closed",slot->directory); }
    return complete;
}
/** Acknowledged copies may be incomplete, but their private directory identity must remain exact and contain no saved originals. */
static bool inspect(RelocationTransaction *transaction, RelocationTransactionSlot *slot) {
    struct stat metadata;
    if (lstat(slot->directory,&metadata) != 0) {
        if (errno == ENOENT) { slot->created = false; return true; }
        return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A registered preparation directory could not be inspected",slot->directory);
    }
    if (!S_ISDIR(metadata.st_mode) || metadata.st_uid != geteuid() || (metadata.st_mode&0077) != 0
        || (slot->acknowledged && (metadata.st_dev != slot->metadata.st_dev || metadata.st_ino != slot->metadata.st_ino))) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A registered preparation directory has foreign identity or ownership",slot->directory);
    }
    if (!contents(transaction,slot,!slot->acknowledged)) { return false; }
    if (!slot->acknowledged) { slot->metadata = metadata; }
    if (lstat(slot->backup,&metadata) == 0 || errno != ENOENT) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"Preparation-only recovery unexpectedly contains saved originals",slot->backup);
    }
    return true;
}
/** Reject reservations that could retire public source/settings/output locations even if an invalid journal claims them. */
static bool private_path(RelocationTransaction *transaction, const RelocationTransactionSlot *slot) {
    const RelocationReport *report = &transaction->report;
    if (relocation_contains(slot->directory,report->source) || relocation_contains(slot->directory,report->destination)
        || relocation_contains(report->source,slot->directory) || relocation_contains(report->destination,slot->directory)) { return false; }
    for (const RelocationReportedFile *row = report->files; row != NULL; row = row->next) {
        if (relocation_contains(slot->directory,row->before) || relocation_contains(slot->directory,row->after)) { return false; }
    }
    return true;
}
/** Validate every registered slot before deleting any; the initial journal itself remains a persistent abort receipt. */
bool relocation_recovery_preparation(RelocationRecovery *recovery) {
    RelocationTransaction *transaction = &recovery->transaction;
    for (RelocationTransactionSlot *slot = transaction->slots; slot != NULL; slot = slot->next) {
        if (slot == transaction->recovery_slot) { continue; }
        if (!private_path(transaction,slot)) {
            return relocation_transaction_fail(transaction,RELOCATION_INVALID_JOURNAL,"A preparation reservation overlaps public project data",slot->directory);
        }
        if (!relocation_recovery_directory(transaction,slot->parent) || !inspect(transaction,slot)) { return false; }
    }
    return relocation_transaction_cleanup(transaction) && relocation_transaction_abandoned(transaction);
}
