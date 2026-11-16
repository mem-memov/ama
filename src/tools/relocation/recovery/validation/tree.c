/** @file
 * @brief Prove prepared tree names are the exact component-wise source/destination projection.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * does not let malformed journal rows authorize unrelated paths.
 */
#include <string.h>
#include "../internal.h"
#include "../../internal.h"

/** Compare a projected suffix without allocating, normalizing or searching source bytes. */
static bool projected(const char *root, const char *suffix, const char *path) {
    size_t length = strlen(root);
    return strncmp(root,path,length) == 0 && strcmp(path+length,suffix) == 0;
}
/** Tree rows are distinct and rooted; immediate parents must precede children as in the original breadth-first preparation. */
bool relocation_recovery_tree_shape(RelocationRecovery *recovery, const RelocationTransactionAction *action) {
    RelocationJournalReader reader = {.recovery=recovery,.source=recovery->record};
    const RelocationReport *report = &recovery->transaction.report;
    if (action->kind != RELOCATION_ACTION_CREATE) {
        return action->tree_count == 0 || relocation_journal_invalid(&reader,"Only destination creation may contain a prepared tree");
    }
    if (action->tree_count == 0) { return relocation_journal_invalid(&reader,"Destination creation lacks its prepared root"); }
    if (action->tree[0].metadata.st_dev != action->prepared_metadata.st_dev
        || action->tree[0].metadata.st_ino != action->prepared_metadata.st_ino
        || action->tree[0].metadata.st_mode != action->prepared_metadata.st_mode) {
        return relocation_journal_invalid(&reader,"Destination creation disagrees with its prepared tree root identity");
    }
    for (size_t index = 0; index < action->tree_count; index++) {
        const RelocationPreparedEntry *entry = &action->tree[index];
        const RelocationStagedFile *file = entry->file;
        if (!relocation_recovery_path(file->before) || !relocation_recovery_path(file->after)
            || !relocation_recovery_path(entry->path) || !relocation_contains(report->source,file->before)) {
            return relocation_journal_invalid(&reader,"A prepared tree row is outside the selected source");
        }
        const char *suffix = file->before+strlen(report->source);
        if (!projected(report->destination,suffix,file->after) || !projected(action->slot->prepared,suffix,entry->path)
            || (S_ISDIR(file->metadata.st_mode) != S_ISDIR(entry->metadata.st_mode)
                || S_ISREG(file->metadata.st_mode) != S_ISREG(entry->metadata.st_mode)
                || S_ISLNK(file->metadata.st_mode) != S_ISLNK(entry->metadata.st_mode))) {
            return relocation_journal_invalid(&reader,"A prepared tree row violates physical projection or entry type");
        }
        if (index == 0) {
            if (*suffix != 0 || !S_ISDIR(entry->metadata.st_mode)) {
                return relocation_journal_invalid(&reader,"The prepared tree must start with its directory root");
            }
        } else {
            const char *slash = strrchr(file->before,'/');
            size_t length = (size_t)(slash-file->before);
            bool parent = false;
            for (size_t previous = 0; previous < index; previous++) {
                const RelocationStagedFile *earlier = action->tree[previous].file;
                if (strcmp(earlier->before,file->before) == 0) {
                    return relocation_journal_invalid(&reader,"A prepared tree contains a duplicate physical entry");
                }
                if (strlen(earlier->before) == length && strncmp(earlier->before,file->before,length) == 0
                    && S_ISDIR(earlier->metadata.st_mode)) { parent = true; }
            }
            if (!parent) { return relocation_journal_invalid(&reader,"A prepared tree entry lacks an earlier directory parent"); }
        }
        bool reported = false;
        for (const RelocationReportedFile *row = report->files; row != NULL; row = row->next) {
            if (strcmp(row->before,file->before) == 0 && strcmp(row->after,file->after) == 0
                && row->mode == file->metadata.st_mode && row->existed && !row->invalidated) { reported = true; }
        }
        if (!reported) { return relocation_journal_invalid(&reader,"A prepared tree entry is absent from the physical report"); }
    }
    for (const RelocationReportedFile *row = report->files; row != NULL; row = row->next) {
        if (!row->existed || row->invalidated || !relocation_contains(report->source,row->before)) { continue; }
        bool found = false;
        for (size_t index = 0; index < action->tree_count; index++) {
            if (strcmp(row->before,action->tree[index].file->before) == 0) { found = true; break; }
        }
        if (!found) { return relocation_journal_invalid(&reader,"A reported selected entry is missing from the prepared tree"); }
    }
    return true;
}
