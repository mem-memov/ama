/** @file
 * @brief Compare actual physical/name removal sets with their exact authorized contribution union.
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * applies one original cut to exactly the selected source and replaced targets,
 * never extra dependency originals or an unreviewed subset.
 */
#include "internal.h"
#include <string.h>
/** Null is the empty set; the admitted selection's arrays retain exact keys. */
static bool contains(const SourceRemovalSelection *cut, const char *key, bool names) {
    if (!cut) { return false; }
    size_t count = names ? cut->identity_count : cut->path_count;
    const char *const *keys = names ? cut->identities : cut->paths;
    for (size_t i = 0; i < count; i++) { if (!strcmp(keys[i], key)) { return true; } }
    return false;
}
/** Each contributed key must occur in the actual reviewed union. */
static bool subset(const SourceRemovalSelection *part, const SourceRemovalSelection *actual,
    bool names, SourceOperationIntentIssue *issue) {
    if (!part) { return true; }
    size_t count = names ? part->identity_count : part->path_count;
    const char *const *keys = names ? part->identities : part->paths;
    for (size_t i = 0; i < count; i++) {
        if (!contains(actual, keys[i], names)) { issue->path = keys[i]; issue->selection = part; return false; }
    }
    return true;
}
/** Both set inclusions prove union equality without ordering or count assumptions. */
static bool equal(const SourceOperationIntent *intent, const SourceRemovalSelection *actual,
    bool names, SourceOperationIntentIssue *issue) {
    if (!subset(intent->source_removal, actual, names, issue)) { return false; }
    for (size_t i = 0; i < intent->replacement_count; i++) {
        if (!subset(intent->replacements[i], actual, names, issue)) { return false; }
    }
    if (!actual) { return true; }
    size_t count = names ? actual->identity_count : actual->path_count;
    const char *const *keys = names ? actual->identities : actual->paths;
    for (size_t i = 0; i < count; i++) {
        if (contains(intent->source_removal, keys[i], names)) { continue; }
        size_t j = 0;
        while (j < intent->replacement_count && !contains(intent->replacements[j], keys[i], names)) { j++; }
        if (j == intent->replacement_count) { issue->path = keys[i]; issue->selection = actual; return false; }
    }
    return true;
}
SourceOperationIntentStatus source_operation_intent_union(const SourceOperationPlan *plan,
    const SourceOperationIntent *intent, SourceOperationIntentIssue *issue) {
    return equal(intent, plan->inputs.removal, false, issue) && equal(intent, plan->inputs.removal, true, issue)
        ? SOURCE_OPERATION_INTENT_OK : SOURCE_OPERATION_INTENT_REMOVAL;
}
