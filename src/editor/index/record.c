/** @file
 * @brief Retain exact source spellings and reject inconsistent duplicate bindings.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) preserves source-backed diagnostics;
 * [ARCH-06](../../../spec/implementation/compiler.md#arch-06) requires exact algorithms.
 */
#include <limits.h>
#include <string.h>
#include "../analysis/internal.h"
/** Append one resolved spelling; the final stable merge removes only identical semantic records. Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04), [ARCH-06](../../../spec/implementation/compiler.md#arch-06). */
bool editor_index_add(EditorAnalysis *analysis, EditorOccurrence value) {
    SourceSpan span = value.spelling;
    if (span.source == NULL || span.start == span.end) { return true; }
    if (!source_span_valid(span)) { return false; }
    EditorOccurrence *record = source_arena_allocate(&analysis->arena, sizeof(*record));
    if (record == NULL) {
        analysis->diagnostic = (Diagnostic){.code=DIAGNOSTIC_OUT_OF_MEMORY,.message="Editor index allocation failed"};
        return false;
    }
    if (value.symbol) {
        value.original = value.symbol->original;
        value.module = value.symbol->owner->module;
        if (value.symbol->owner->specialization) { value.runtime = value.symbol->owner->specialization->scope->runtime; }
    } else if (value.static_symbol) { value.module = value.static_symbol->owner->definition; }
    *record = value;
    record->next = record->alternatives = NULL;
    if (analysis->last == NULL) { analysis->first = record; }
    else { analysis->last->next = record; }
    analysis->last = record;
    return true;
}
/** Exact physical path and original half-open coordinates define a deterministic source order. Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04), [ARCH-06](../../../spec/implementation/compiler.md#arch-06). */
static int compare(const EditorOccurrence *left, const EditorOccurrence *right) {
    int path = strcmp(source_path(left->spelling.source), source_path(right->spelling.source));
    if (path != 0) { return path; }
    if (left->spelling.start != right->spelling.start) { return left->spelling.start < right->spelling.start ? -1 : 1; }
    return (left->spelling.end > right->spelling.end) - (left->spelling.end < right->spelling.end);
}
/** Merge sorted finite runs without recursion or fresh allocation, consuming only their next links. Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04), [ARCH-06](../../../spec/implementation/compiler.md#arch-06). */
static EditorOccurrence *merge(EditorOccurrence *left, EditorOccurrence *right) {
    EditorOccurrence head = {0}, *tail = &head;
    while (left != NULL && right != NULL) {
        EditorOccurrence **chosen = compare(left, right) <= 0 ? &left : &right;
        tail->next = *chosen;
        *chosen = (*chosen)->next;
        tail = tail->next;
    }
    tail->next = left == NULL ? right : left;
    return head.next;
}
/** Exact duplicate evidence contributes one target, while distinct concrete
 * symbols and distinct foreign origins remain alternatives at the same span.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) preserves every accepted context rather than selecting its first projection.
 */
static bool same_target(const EditorOccurrence *left, const EditorOccurrence *right) {
    return left->symbol == right->symbol && left->type == right->type
        && left->static_symbol == right->static_symbol && left->module == right->module
        && left->original == right->original && left->runtime == right->runtime;
}

/** Binary merge bins bound stack space by size_t bit count; every record moves O(log R) times. Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04), [ARCH-06](../../../spec/implementation/compiler.md#arch-06). */
bool editor_index_order(EditorAnalysis *analysis) {
    EditorOccurrence *bins[sizeof(size_t) * CHAR_BIT] = {0};
    while (analysis->first != NULL) {
        EditorOccurrence *run = analysis->first;
        analysis->first = run->next;
        run->next = NULL;
        size_t level = 0;
        while (level < sizeof(bins)/sizeof(*bins) && bins[level] != NULL) {
            run = merge(bins[level], run);
            bins[level++] = NULL;
        }
        if (level == sizeof(bins)/sizeof(*bins)) { return false; }
        bins[level] = run;
    }
    for (size_t level = 0; level < sizeof(bins)/sizeof(*bins); level++) {
        analysis->first = merge(bins[level], analysis->first);
    }
    for (EditorOccurrence *record = analysis->first; record != NULL; record = record->next) {
        EditorOccurrence *tail = record;
        while (record->next != NULL && compare(record, record->next) == 0) {
            EditorOccurrence *duplicate = record->next;
            if (record->spelling.source != duplicate->spelling.source) {
                analysis->diagnostic = (Diagnostic){.code=DIAGNOSTIC_TYPE_MISMATCH,
                    .message="Editor index found conflicting source owners",.primary={.span=record->spelling}};
                return false;
            }
            record->declaration = record->declaration || duplicate->declaration;
            record->next = duplicate->next;
            duplicate->next = NULL;
            EditorOccurrence *prior = record;
            while (prior && !same_target(prior, duplicate)) { prior = prior->alternatives; }
            if (prior) { prior->declaration = prior->declaration || duplicate->declaration; }
            else { tail->alternatives = duplicate; tail = duplicate; }
        }
        analysis->last = record;
    }
    return true;
}
