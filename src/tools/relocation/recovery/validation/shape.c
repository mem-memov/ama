/** @file
 * @brief Validate complete report/action/slot geometry before trusting a recovery journal.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * ties each recovery mutation to its explicit move, rewrite or invalidation record.
 */
#include <string.h>
#include <unistd.h>
#include "../internal.h"
#include "../../internal.h"

/** A slot is a private direct child of its recorded parent, with two exact reserved child names. */
bool relocation_recovery_slot_shape(const RelocationTransactionSlot *owned, bool acknowledged) {
    if (!relocation_recovery_path(owned->directory) || !relocation_recovery_path(owned->parent)
        || !relocation_recovery_path(owned->prepared) || !relocation_recovery_path(owned->backup)) { return false; }
    const char *slash = strrchr(owned->directory,'/');
    size_t parent_length = slash == owned->directory ? 1 : (size_t)(slash-owned->directory);
    size_t directory_length = strlen(owned->directory);
    return strlen(owned->parent) == parent_length && strncmp(owned->parent,owned->directory,parent_length) == 0
        && strncmp(slash+1,".ama-relocation-",16) == 0 && strlen(slash+1) > 16
        && strncmp(owned->directory,owned->prepared,directory_length) == 0
        && strcmp(owned->prepared+directory_length,"/prepared") == 0
        && strncmp(owned->directory,owned->backup,directory_length) == 0
        && strcmp(owned->backup+directory_length,"/original") == 0
        && (!acknowledged || (S_ISDIR(owned->metadata.st_mode) && (owned->metadata.st_mode&0077) == 0
            && owned->metadata.st_uid == geteuid()));
}
/** Report settings are data rather than recovery authority, but malformed source/root identities still invalidate the receipt. */
bool relocation_recovery_report_shape(const RelocationReport *report) {
    if (!relocation_recovery_path(report->source) || !relocation_recovery_path(report->destination)
        || strcmp(report->source,"/") == 0 || strcmp(report->destination,"/") == 0
        || relocation_contains(report->source,report->destination) || relocation_contains(report->destination,report->source)
        || !report->rebuild_required || (report->entry != NULL && !module_path_valid(report->entry))) { return false; }
    for (size_t index = 0; index < report->before_root_count; index++) {
        if (!relocation_recovery_path(report->before_roots[index].path)) { return false; }
    }
    for (size_t index = 0; index < report->after_root_count; index++) {
        if (!relocation_recovery_path(report->after_roots[index].path)) { return false; }
    }
    for (const RelocationReportedModule *row = report->modules; row != NULL; row = row->next) {
        if (!relocation_recovery_path(row->before) || !relocation_recovery_path(row->after)
            || (row->before_identity != NULL && !module_path_valid(row->before_identity))
            || !module_path_valid(row->after_identity)) { return false; }
    }
    for (const RelocationReportedFile *row = report->files; row != NULL; row = row->next) {
        if (!relocation_recovery_path(row->before) || !relocation_recovery_path(row->after)
            || (!row->existed && (!row->invalidated || row->mode != 0))) { return false; }
    }
    return true;
}
/** Later actions may only replace reported rewritten regular files or retire explicit existing outputs. */
static bool external(const RelocationTransaction *transaction, const RelocationTransactionAction *action) {
    if (action->kind == RELOCATION_ACTION_CREATE || relocation_contains(transaction->report.source,action->path)
        || relocation_contains(transaction->report.destination,action->path)) { return false; }
    for (const RelocationReportedFile *row = transaction->report.files; row != NULL; row = row->next) {
        if (strcmp(row->before,action->path) != 0 || !row->existed || row->mode != action->original_metadata.st_mode) { continue; }
        if (action->kind == RELOCATION_ACTION_REMOVE) { return row->invalidated; }
        return row->rewritten && !row->invalidated && S_ISREG(action->original_metadata.st_mode)
            && S_ISREG(action->prepared_metadata.st_mode) && action->file != NULL && action->file->replacement != NULL;
    }
    return false;
}
/** Pairwise finite checks are exact; nothing is mutated or inferred from a timestamp or path substring. */
bool relocation_journal_validate(RelocationRecovery *recovery) {
    RelocationTransaction *transaction = &recovery->transaction;
    RelocationJournalReader reader = {.recovery=recovery,.source=recovery->record};
    if (!relocation_recovery_report_shape(&transaction->report) || transaction->recovery_slot == NULL) {
        return relocation_journal_invalid(&reader,"The journal report contains invalid physical or logical scope");
    }
    const RelocationTransactionAction *first = transaction->first;
    if (first == NULL || first->next == NULL || first->kind != RELOCATION_ACTION_REMOVE
        || strcmp(first->path,transaction->report.source) != 0 || !S_ISDIR(first->original_metadata.st_mode)
        || first->next->kind != RELOCATION_ACTION_CREATE || strcmp(first->next->path,transaction->report.destination) != 0) {
        return relocation_journal_invalid(&reader,"The journal lacks its ordered source removal and destination creation");
    }
    for (const RelocationTransactionSlot *owned = transaction->slots; owned != NULL; owned = owned->next) {
        if (!relocation_recovery_slot_shape(owned,true)) { return relocation_journal_invalid(&reader,"A private recovery slot has invalid spelling or ownership metadata"); }
        for (const RelocationTransactionSlot *other = owned->next; other != NULL; other = other->next) {
            if (relocation_contains(owned->directory,other->directory) || relocation_contains(other->directory,owned->directory)) {
                return relocation_journal_invalid(&reader,"Private recovery slots overlap");
            }
        }
        for (const RelocationTransactionAction *action = first; action != NULL; action = action->next) {
            if (relocation_contains(action->path,owned->directory) || relocation_contains(owned->directory,action->path)) {
                return relocation_journal_invalid(&reader,"A recovery slot overlaps a public action path");
            }
        }
    }
    for (const RelocationTransactionAction *action = first; action != NULL; action = action->next) {
        if (!relocation_recovery_path(action->path)) { return relocation_journal_invalid(&reader,"A public recovery action path is not canonical"); }
        const char *slash = strrchr(action->path,'/');
        size_t length = slash == action->path ? 1 : (size_t)(slash-action->path);
        if (strlen(action->slot->parent) != length || strncmp(action->path,action->slot->parent,length) != 0
            || (action != first && action != first->next && !external(transaction,action))) {
            return relocation_journal_invalid(&reader,"An action is not tied to its recorded parent or reported operation");
        }
        for (const RelocationTransactionAction *other = action->next; other != NULL; other = other->next) {
            if (relocation_contains(action->path,other->path) || relocation_contains(other->path,action->path)) {
                return relocation_journal_invalid(&reader,"Public recovery actions overlap");
            }
        }
        if (!relocation_recovery_tree_shape(recovery,action)) { return false; }
    }
    return true;
}
