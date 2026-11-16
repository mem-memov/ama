/** @file
 * @brief Grant replay authority only after validating every retained private slot and inverse precondition.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains foreign or changed state instead of overwriting it during recovery.
 */
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include "../internal.h"

/** A directory owned by the recorded identity may contain only its reserved transaction children. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool slot(RelocationTransaction *transaction, RelocationTransactionSlot *owned) {
    RelocationPathState state = relocation_transaction_probe(transaction,owned->directory,&owned->metadata);
    if (state == RELOCATION_PATH_ABSENT && owned != transaction->recovery_slot) { owned->created = false; return true; }
    if (state != RELOCATION_PATH_EXPECTED) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A recovery slot no longer has its recorded physical identity",owned->directory);
    }
    struct stat metadata;
    if (lstat(owned->directory,&metadata) != 0 || metadata.st_uid != owned->metadata.st_uid || (metadata.st_mode&0077) != 0) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A recovery slot no longer has private ownership",owned->directory);
    }
    DIR *directory = opendir(owned->directory);
    if (directory == NULL) { return relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A recovery slot could not be enumerated",owned->directory); }
    bool complete = true;
    for (;;) {
        errno = 0;
        struct dirent *entry = readdir(directory);
        if (entry == NULL) {
            if (errno != 0) { complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"Recovery slot enumeration failed",owned->directory); }
            break;
        }
        if (strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0) { continue; }
        bool allowed = owned == transaction->recovery_slot ? (strcmp(entry->d_name,"plan") == 0 || strcmp(entry->d_name,"intent") == 0)
            : strcmp(entry->d_name,"prepared") == 0 || strcmp(entry->d_name,"original") == 0;
        if (!allowed) {
            complete = relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A recovery slot contains an unrecorded child",owned->directory);
            break;
        }
    }
    if (closedir(directory) != 0) { complete = relocation_transaction_fail(transaction,RELOCATION_IO_ERROR,"A recovery slot directory could not be closed",owned->directory); }
    return complete;
}
/** Originals must remain in exactly one location; replacements can be private or published, never foreign. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool action(RelocationRecovery *recovery, const RelocationTransactionAction *item) {
    RelocationTransaction *transaction = &recovery->transaction;
    RelocationPathState saved = relocation_transaction_probe(transaction,item->slot->backup,&item->original_metadata);
    RelocationPathState prepared = relocation_transaction_probe(transaction,item->slot->prepared,&item->prepared_metadata);
    if (item->kind == RELOCATION_ACTION_CREATE) {
        if (saved != RELOCATION_PATH_ABSENT) {
            return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"Destination creation has an unexpected saved original",item->slot->backup);
        }
    } else if (saved != RELOCATION_PATH_ABSENT && saved != RELOCATION_PATH_EXPECTED) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"A retained original no longer has its recorded identity",item->slot->backup);
    }
    if (item->kind == RELOCATION_ACTION_REMOVE ? prepared != RELOCATION_PATH_ABSENT
        : prepared != RELOCATION_PATH_ABSENT && prepared != RELOCATION_PATH_EXPECTED) {
        return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"Private prepared state conflicts with the journal",item->slot->prepared);
    }
    if (transaction->committed) { return true; }
    RelocationPathState original = item->kind == RELOCATION_ACTION_CREATE ? RELOCATION_PATH_ABSENT
        : relocation_transaction_probe(transaction,item->path,&item->original_metadata);
    RelocationPathState published = item->kind == RELOCATION_ACTION_REMOVE ? RELOCATION_PATH_ABSENT
        : relocation_transaction_probe(transaction,item->path,&item->prepared_metadata);
    if (item->kind != RELOCATION_ACTION_CREATE) {
        bool restored = original == RELOCATION_PATH_EXPECTED && saved == RELOCATION_PATH_ABSENT;
        bool retained = saved == RELOCATION_PATH_EXPECTED
            && (original == RELOCATION_PATH_ABSENT || published == RELOCATION_PATH_EXPECTED);
        if (!restored && !retained) {
            return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"The saved original cannot be restored without losing existing state",item->path);
        }
    }
    if (item->kind != RELOCATION_ACTION_REMOVE) {
        bool absent = published == RELOCATION_PATH_ABSENT;
        bool restored = item->kind == RELOCATION_ACTION_REPLACE && original == RELOCATION_PATH_EXPECTED;
        if (!absent && !restored && (published != RELOCATION_PATH_EXPECTED || prepared != RELOCATION_PATH_ABSENT)) {
            return relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"The published path conflicts with its prepared replacement",item->path);
        }
        if (published == RELOCATION_PATH_EXPECTED && !relocation_recovery_replacement(recovery,item)) { return false; }
    }
    return true;
}
/** Acquire every mutation parent before validating objects; no inverse starts on a partly checked plan. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_recovery_preflight(RelocationRecovery *recovery) {
    RelocationTransaction *transaction = &recovery->transaction;
    for (const RelocationTransactionAction *item = transaction->first; item != NULL; item = item->next) {
        if (!relocation_recovery_directory(transaction,item->slot->parent)) { return false; }
    }
    for (RelocationTransactionSlot *owned = transaction->slots; owned != NULL; owned = owned->next) {
        if (!slot(transaction,owned)) { return false; }
    }
    for (const RelocationTransactionAction *item = transaction->first; item != NULL; item = item->next) {
        if (!action(recovery,item)) { return false; }
    }
    return true;
}
