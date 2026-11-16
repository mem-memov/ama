/** @file
 * @brief Compare complete independently captured plans by exact identities and bytes, never hashes or timestamp guesses.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rejects changed inputs instead of overwriting a stale plan.
 */
#include <string.h>
#include "../internal.h"
#include "../../staging/internal.h"

/** Nullable path/settings text retains null as absence, distinct from an empty value. */
static bool same_name(const char *left, const char *right) {
    return left == NULL || right == NULL ? left == right : strcmp(left,right) == 0;
}
/** Byte equality includes NUL, literal link text and original line endings without decoding. */
static bool same_text(const SourceText *left, const SourceText *right) {
    if (left == NULL || right == NULL) { return left == right; }
    return source_length(left) == source_length(right)
        && memcmp(source_bytes(left),source_bytes(right),source_length(left)) == 0;
}
/** Match all physical inputs by their exact canonical original path and compare both original and prepared states. */
static bool files_equal(const RelocationStaging *before, const RelocationStaging *current) {
    size_t expected = 0, actual = 0;
    for (const RelocationStagedFile *file = before->files; file != NULL; file = file->next) {
        const RelocationStagedFile *matched = current->files;
        while (matched != NULL && strcmp(matched->before,file->before) != 0) { matched = matched->next; }
        if (matched == NULL || !same_name(file->after,matched->after)
            || !relocation_staging_metadata(&file->metadata,&matched->metadata)
            || file->protected_input != matched->protected_input
            || !same_text(file->original,matched->original) || !same_text(file->replacement,matched->replacement)) { return false; }
        expected++;
    }
    for (const RelocationStagedFile *file = current->files; file != NULL; file = file->next) { actual++; }
    return expected == actual;
}
/** Added/deleted candidates and changed root-kind mappings invalidate even an unchanged selected source tree. */
static bool modules_equal(const RelocationPreview *before, const RelocationPreview *current) {
    size_t expected = 0, actual = 0;
    for (const RelocationModule *module = before->modules; module != NULL; module = module->next) {
        const RelocationModule *matched = current->modules;
        while (matched != NULL && strcmp(matched->before_path,module->before_path) != 0) { matched = matched->next; }
        if (matched == NULL || !same_name(module->after_path,matched->after_path)
            || !same_name(module->before_identity,matched->before_identity)
            || !same_name(module->after_identity,matched->after_identity) || module->active != matched->active
            || module->device != matched->device || module->inode != matched->inode) { return false; }
        expected++;
    }
    for (const RelocationModule *module = current->modules; module != NULL; module = module->next) { actual++; }
    return expected == actual;
}
/** Existing output identity and absent-output observations are rechecked before any output retirement. */
static bool outputs_equal(const RelocationStaging *before, const RelocationStaging *current) {
    size_t expected = 0, actual = 0;
    for (const RelocationInvalidation *output = before->invalidations; output != NULL; output = output->next) {
        const RelocationInvalidation *matched = current->invalidations;
        while (matched != NULL && strcmp(matched->before,output->before) != 0) { matched = matched->next; }
        if (matched == NULL || !same_name(output->after,matched->after) || output->existed != matched->existed
            || (output->existed && !relocation_staging_metadata(&output->metadata,&matched->metadata))) { return false; }
        expected++;
    }
    for (const RelocationInvalidation *output = current->invalidations; output != NULL; output = output->next) { actual++; }
    return expected == actual;
}
/** Independent final graph validation has already succeeded; exact plan comparison rejects all changed contributing records. */
bool relocation_transaction_same_plan(RelocationTransaction *transaction, const RelocationStaging *current) {
    const RelocationStaging *before = transaction->staging;
    return (same_name(before->entry,current->entry) && files_equal(before,current)
        && modules_equal(before->references->preview,current->references->preview) && outputs_equal(before,current))
        || relocation_transaction_fail(transaction,RELOCATION_CHANGED_INPUT,"The project no longer matches the validated relocation plan",
            before->references->preview->source);
}
