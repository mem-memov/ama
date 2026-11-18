/**
 * @file
 * @brief Convert explicit foreign formal trees into owned finite named signatures.
 *
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) makes signature
 * names foreign slots and fixes categories structurally. This traversal neither
 * imports those names nor inspects an actual specialization.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) keeps partial
 * trees in the inventory arena and uses explicit traversal frames.
 */
#include <assert.h>
#include <stdint.h>
#include "signature.h"
#include "../internal.h"

/** A frame consumes one sibling list into its uniquely owned signature.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
typedef struct SignatureBuildWork {
    struct SignatureBuildWork *previous;
    const AstStaticFormal *next;
    StaticSignature *signature;
} SignatureBuildWork;

/** Prepare a child signature/frame before publishing its internal destination.
 * Every frame owns a finite, unconsumed AST sibling suffix.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
static SignatureBuildWork *push(StaticInventory *inventory, SignatureBuildWork *previous,
    const AstStaticFormal *first, StaticSignature **destination) {
    StaticSignature *signature = static_allocate(inventory, sizeof(*signature));
    SignatureBuildWork *work = static_allocate(inventory, sizeof(*work));
    if (signature == NULL || work == NULL) { return NULL; }
    *work = (SignatureBuildWork){previous, first, signature};
    *destination = signature;
    return work;
}

/** Reject a duplicate at its later exact header, retaining the original foreign
 * header rather than pretending it belongs to the module's local symbol table.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
static bool distinct(StaticInventory *inventory, const StaticSignature *signature,
    SourceSpan name) {
    for (const StaticSignatureEntry *entry = signature->first; entry != NULL; entry = entry->next) {
        if (!static_same_name(entry->name, name)) { continue; }
        static_invalid(inventory, name, "Duplicate constructor signature slot", NULL);
        inventory->related = (DiagnosticRelated){.location = {.span = entry->name},
            .role = "previous constructor signature slot"};
        inventory->error.related = &inventory->related;
        inventory->error.related_count = 1;
        return false;
    }
    return true;
}

/** Advance a frame before descending: each AST node is consumed exactly once.
 * A successful root is published only after every descendant passes validation.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
bool static_signature_build(StaticInventory *inventory, const AstStaticFormal *first,
    StaticSignature **output) {
    if (inventory->status != STATIC_OK) { return false; }
    assert(first != NULL && output != NULL);
    StaticSignature *root = NULL;
    SignatureBuildWork *work = push(inventory, NULL, first, &root);
    if (work == NULL) { return false; }
    while (work != NULL) {
        const AstStaticFormal *formal = work->next;
        if (formal == NULL) { work = work->previous; continue; }
        work->next = formal->next;
        StaticSignature *signature = work->signature;
        if (!distinct(inventory, signature, formal->name)) { return false; }
        if (signature->count == SIZE_MAX) { return static_memory_failure(inventory); }
        StaticSignatureEntry *entry = static_allocate(inventory, sizeof(*entry));
        if (entry == NULL) { return false; }
        entry->name = formal->name;
        entry->category = formal->first == NULL ? STATIC_CATEGORY_TYPE : STATIC_CATEGORY_TEMPLATE;
        if (signature->last == NULL) { signature->first = entry; }
        else { signature->last->next = entry; }
        signature->last = entry;
        signature->count++;
        if (formal->first != NULL) {
            work = push(inventory, work, formal->first, &entry->signature);
            if (work == NULL) { return false; }
        }
    }
    *output = root;
    return true;
}
