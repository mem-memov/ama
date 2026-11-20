/** @file
 * @brief Preserve every source-only and concrete binding alternative after exact name edits.
 * [TPL-02](../../../../../spec/language/templates.md#tpl-02) retains original scopes;
 * [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06) requires graph preservation.
 */
#include <string.h>
#include "internal.h"
/** Optional original modules retain their complete logical origin.
 * [TPL-02](../../../../../spec/language/templates.md#tpl-02) forbids name-only scope merging.
 */
static bool module(const ProjectModule *old, const ProjectModule *fresh) {
    return old == NULL ? fresh == NULL : fresh != NULL
        && strcmp(old->source->logical_path,fresh->source->logical_path) == 0;
}
/** Compare source anchors even when no runtime specialization exists.
 * [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06) checks all accepted binding evidence.
 */
static bool target(const RenameIdentity *check, const EditorOccurrence *old, const EditorOccurrence *fresh) {
    if (!module(old->module,fresh->module)
        || !rename_same_symbol(check,old->symbol,fresh->symbol)
        || !rename_same_type(check,old->type,fresh->type)) { return false; }
    if (old->static_symbol == NULL ? fresh->static_symbol != NULL : fresh->static_symbol == NULL
        || old->static_symbol->role != fresh->static_symbol->role
        || old->static_symbol->ordinal != fresh->static_symbol->ordinal
        || !module(old->static_symbol->owner->definition,fresh->static_symbol->owner->definition)
        || !rename_same_span(check,old->static_symbol->site,fresh->static_symbol->site)
        || !rename_same_span(check,old->static_symbol->name,fresh->static_symbol->name)) { return false; }
    if (old->original == NULL ? fresh->original != NULL : fresh->original == NULL
        || old->original->role != fresh->original->role
        || !rename_same_span(check,old->original->name,fresh->original->name)) { return false; }
    return true;
}
/** Distinct alternatives form a finite set: require exactly one mate in both directions.
 * [ARCH-06](../../../../../spec/implementation/compiler.md#arch-06) never aligns discovery order.
 */
static bool subset(const RenameIdentity *check, const EditorOccurrence *old, const EditorOccurrence *fresh, bool reverse) {
    for (const EditorOccurrence *a = old; a; a = a->alternatives) {
        size_t matches = 0;
        for (const EditorOccurrence *b = fresh; b; b = b->alternatives) {
            if (reverse ? target(check,b,a) : target(check,a,b)) { matches++; }
        }
        if (matches != 1) { return false; }
    }
    return true;
}
bool rename_identity_occurrences(RenameIdentity *check) {
    const EditorOccurrence *old = check->plan->workspace->first, *fresh = check->plan->candidate.first;
    while (old && fresh) {
        if (old->declaration != fresh->declaration || !rename_same_span(check,old->spelling,fresh->spelling)
            || !subset(check,old,fresh,false) || !subset(check,fresh,old,true)) { return false; }
        old = old->next;
        fresh = fresh->next;
    }
    return old == NULL && fresh == NULL;
}
