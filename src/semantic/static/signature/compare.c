/**
 * @file
 * @brief Compare complete finite signature name sets and recursive categories without source-order bias.
 *
 * [TPL-07](../../../../spec/language/templates.md#tpl-07) requires recursive
 * named equality. [TPL-12](../../../../spec/language/static-results.md#tpl-12)
 * permits a supplied root subset when outputs request later closure. A deferred entry is an obligation, never a guessed Type or
 * wildcard. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * gives explicit work records inventory lifetime, including failed comparisons.
 */
#include <assert.h>
#include "signature.h"
#include "../internal.h"

/** A pair of finite acyclic signatures still requiring comparison.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
typedef struct SignatureCompareWork {
    struct SignatureCompareWork *next;
    struct SignatureCompareWork *memo_next;
    const StaticSignature *left;
    const StaticSignature *right;
    bool provided;
} SignatureCompareWork;

/** Exact whole-name lookup is independent of foreign declaration order.
 * Component contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
static const StaticSignatureEntry *find(const StaticSignature *signature, SourceSpan name) {
    for (const StaticSignatureEntry *entry = signature->first; entry != NULL; entry = entry->next) {
        if (static_same_name(entry->name, name)) { return entry; }
    }
    return NULL;
}

/** Queue each exact ordered pair once. Memoization avoids exponential repeated
 * expansion of shared acyclic subgraphs; pointer identity caches work rather
 * than defining semantic equality. Root subset mode is part of the work key.
 * No caller temporary state is retained.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
static bool push(StaticInventory *inventory, SignatureCompareWork **work, SignatureCompareWork **memo,
    const StaticSignature *left, const StaticSignature *right, bool provided) {
    assert(left != NULL && right != NULL);
    for (const SignatureCompareWork *prior = *memo; prior != NULL; prior = prior->memo_next) {
        if (prior->left == left && prior->right == right && prior->provided == provided) { return true; }
    }
    SignatureCompareWork *item = static_allocate(inventory, sizeof(*item));
    if (item == NULL) { return false; }
    *item = (SignatureCompareWork){.next = *work, .memo_next = *memo, .left = left, .right = right, .provided = provided};
    *memo = item;
    *work = item;
    return true;
}

/** Compare all known structure before allowing pending metadata to affect the
 * result. Distinct-name preconditions make equal cardinality plus full left
 * membership sufficient for equality of the complete name sets.
 * Contract: [TPL-07](../../../../spec/language/templates.md#tpl-07).
 */
bool static_signature_compare(StaticInventory *inventory, const StaticSignature *left,
    const StaticSignature *right, StaticSignatureMatch match, StaticSignatureComparison *output) {
    if (inventory->status != STATIC_OK) { return false; }
    assert(output != NULL && (match == STATIC_SIGNATURE_EXACT || match == STATIC_SIGNATURE_PROVIDED));
    SignatureCompareWork *work = NULL, *memo = NULL;
    if (!push(inventory, &work, &memo, left, right, match == STATIC_SIGNATURE_PROVIDED)) { return false; }
    StaticSignatureComparison result = STATIC_SIGNATURE_EQUAL;
    while (work != NULL) {
        const StaticSignature *a = work->left, *b = work->right;
        bool provided = work->provided;
        work = work->next;
        if ((!provided && a->count != b->count) || a->count > b->count) { *output = STATIC_SIGNATURE_DIFFERENT; return true; }
        for (const StaticSignatureEntry *entry = a->first; entry != NULL; entry = entry->next) {
            const StaticSignatureEntry *other = find(b, entry->name);
            if (other == NULL) { *output = STATIC_SIGNATURE_DIFFERENT; return true; }
            if (entry->category == STATIC_CATEGORY_DEFERRED || other->category == STATIC_CATEGORY_DEFERRED) {
                result = STATIC_SIGNATURE_PENDING;
            } else if (entry->category != other->category) {
                *output = STATIC_SIGNATURE_DIFFERENT;
                return true;
            } else if (entry->category == STATIC_CATEGORY_TEMPLATE) {
                if (!push(inventory, &work, &memo, entry->signature, other->signature, false)) { return false; }
            }
        }
    }
    *output = result;
    return true;
}
