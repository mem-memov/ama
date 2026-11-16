/** @file
 * @brief Compare prepared recovery authority with its independently persisted preparation scope and slot registrations.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects mismatched records instead of granting recovery authority to an unrelated prepared plan.
 */
#include <string.h>
#include "../internal.h"

/** Null identity is distinct from an empty identifier; all other text compares byte-for-byte. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool text(const char *left, const char *right) {
    return left == NULL || right == NULL ? left == right : strcmp(left,right) == 0;
}
/** Root order, family and literal exclusions are part of the supplied settings contract. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool roots(const ModuleRoot *left, size_t left_count, const ModuleRoot *right, size_t right_count) {
    if (left_count != right_count) { return false; }
    for (size_t index = 0; index < left_count; index++) {
        if (!text(left[index].path,right[index].path) || left[index].kind != right[index].kind
            || left[index].excluded_count != right[index].excluded_count) { return false; }
        for (size_t item = 0; item < left[index].excluded_count; item++) {
            if (!text(left[index].excluded[item],right[index].excluded[item])) { return false; }
        }
    }
    return true;
}
/** Module and physical role rows retain writer order, so linear equality includes duplicate/missing records. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool report(const RelocationReport *left, const RelocationReport *right) {
    if (!text(left->source,right->source) || !text(left->destination,right->destination) || !text(left->entry,right->entry)
        || left->rebuild_required != right->rebuild_required
        || !roots(left->before_roots,left->before_root_count,right->before_roots,right->before_root_count)
        || !roots(left->after_roots,left->after_root_count,right->after_roots,right->after_root_count)) { return false; }
    const RelocationReportedModule *left_module = left->modules, *right_module = right->modules;
    while (left_module != NULL && right_module != NULL) {
        if (!text(left_module->before,right_module->before) || !text(left_module->after,right_module->after)
            || !text(left_module->before_identity,right_module->before_identity) || !text(left_module->after_identity,right_module->after_identity)
            || left_module->active != right_module->active) { return false; }
        left_module = left_module->next;
        right_module = right_module->next;
    }
    if (left_module != NULL || right_module != NULL) { return false; }
    const RelocationReportedFile *left_file = left->files, *right_file = right->files;
    while (left_file != NULL && right_file != NULL) {
        if (!text(left_file->before,right_file->before) || !text(left_file->after,right_file->after) || left_file->mode != right_file->mode
            || left_file->existed != right_file->existed || left_file->rewritten != right_file->rewritten || left_file->draft != right_file->draft
            || left_file->opaque != right_file->opaque || left_file->invalidated != right_file->invalidated) { return false; }
        left_file = left_file->next;
        right_file = right_file->next;
    }
    return left_file == NULL && right_file == NULL;
}
/** Every acknowledged preparation directory appears exactly once in the plan, with no new or missing ownership claim. Component contract: [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_recovery_same_intent(RelocationRecovery *recovery, const RelocationReport *intent, const RelocationTransactionSlot *slots) {
    RelocationTransaction *transaction = &recovery->transaction;
    RelocationJournalReader reader = {.recovery=recovery,.source=recovery->record};
    if (!report(intent,&transaction->report)) { return relocation_journal_invalid(&reader,"Prepared and preparation reports disagree"); }
    size_t before_count = 0, after_count = 0;
    for (const RelocationTransactionSlot *before = slots; before != NULL; before = before->next) {
        before_count++;
        bool found = false;
        for (const RelocationTransactionSlot *after = transaction->slots; after != NULL; after = after->next) {
            if (text(before->directory,after->directory) && text(before->parent,after->parent)
                && text(before->prepared,after->prepared) && text(before->backup,after->backup)
                && before->acknowledged && before->metadata.st_dev == after->metadata.st_dev
                && before->metadata.st_ino == after->metadata.st_ino && before->metadata.st_mode == after->metadata.st_mode
                && before->metadata.st_uid == after->metadata.st_uid && before->metadata.st_gid == after->metadata.st_gid) { found = true; }
        }
        if (!found) { return relocation_journal_invalid(&reader,"Prepared storage lacks matching acknowledged preparation ownership"); }
    }
    for (const RelocationTransactionSlot *after = transaction->slots; after != NULL; after = after->next) { after_count++; }
    return before_count == after_count || relocation_journal_invalid(&reader,"Prepared storage has unregistered private directories");
}
