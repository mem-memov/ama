/**
 * @file
 * @brief Detect actual cycles among known actual signature edges before recursive comparison.
 *
 * [TPL-02](../../../../spec/language/templates.md#tpl-02) rejects infinite
 * signature dependencies. This is a finite graph check, independent of runtime
 * recursion and specialization growth. [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * requires exact algorithms; active/completed pointer identities prove cycles
 * without depth limits, timeouts or guessed structural matches.
 */
#include "../internal.h"

/** Memoized exact signature identity; active nodes are ancestors of the cursor.
 * Component contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
typedef struct SignatureVisit {
    struct SignatureVisit *next;
    struct SignatureVisit *parent;
    const StaticSignature *signature;
    const StaticSignatureEntry *next_entry;
    bool complete;
} SignatureVisit;

/** Reuse one identity or allocate its unvisited traversal record.
 * Component contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
static SignatureVisit *visit(StaticInventory *inventory, SignatureVisit **records,
    const StaticSignature *signature, bool *fresh) {
    for (SignatureVisit *item = *records; item != NULL; item = item->next) {
        if (item->signature == signature) { *fresh = false; return item; }
    }
    SignatureVisit *item = static_allocate(inventory, sizeof(*item));
    if (item == NULL) { return NULL; }
    *item = (SignatureVisit){.next = *records, .signature = signature, .next_entry = signature->first};
    *records = item;
    *fresh = true;
    return item;
}

/** DFS with explicit parent/entry cursors. Each edge is consumed once, each
 * finite vertex finishes once; an edge to an unfinished vertex closes an actual
 * active ancestor cycle. Deferred entries are not fabricated graph edges.
 * Component contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
static bool check(StaticInventory *inventory, SignatureVisit **records, const StaticSignature *root) {
    if (root == NULL) { return true; }
    bool fresh;
    SignatureVisit *cursor = visit(inventory, records, root, &fresh);
    if (cursor == NULL) { return false; }
    if (!fresh) { return true; }
    while (cursor != NULL) {
        const StaticSignatureEntry *entry = cursor->next_entry;
        if (entry == NULL) { cursor->complete = true; cursor = cursor->parent; continue; }
        cursor->next_entry = entry->next;
        if (entry->category != STATIC_CATEGORY_TEMPLATE) { continue; }
        SignatureVisit *child = visit(inventory, records, entry->signature, &fresh);
        if (child == NULL) { return false; }
        if (!fresh && !child->complete) {
            return static_invalid(inventory, entry->name, "Cyclic constructor signature dependency", NULL);
        }
        if (fresh) { child->parent = cursor; cursor = child; }
    }
    return true;
}

/** Check every published actual metadata root with shared memoization;
 * shared acyclic subtrees are accepted and visited once, not mistaken for cycles.
 * Contract: [TPL-02](../../../../spec/language/templates.md#tpl-02).
 */
bool static_signatures_acyclic(StaticInventory *inventory) {
    if (inventory->status != STATIC_OK) { return false; }
    SignatureVisit *records = NULL;
    for (const StaticModule *module = inventory->first; module != NULL; module = module->next) {
        if (!check(inventory, &records, module->constructor_signature)) { return false; }
        for (const StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
            if (!check(inventory, &records, symbol->signature)) { return false; }
        }
    }
    return true;
}
