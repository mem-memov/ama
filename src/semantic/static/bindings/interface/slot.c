/**
 * @file
 * @brief Append immutable interface slots with exact whole-name uniqueness and checked counts.
 *
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) fixes each
 * snapshot before binding. [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 * prevents partial records from escaping on allocation failure.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"

/** Compare exact borrowed names independently of their source/storage provider.
 * Contract: [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
static bool same(BindingName left, BindingName right) {
    return left.length == right.length && memcmp(left.bytes, right.bytes, left.length) == 0;
}

/** Validate before allocation and link only a complete prepared record. Original
 * ordinals survive filtering; sequence ordinals describe this frozen snapshot.
 * Contract: [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
bool binding_slot_append(BindingPlan *plan, BindingSequence *sequence, BindingSlot prototype) {
    if (plan->status != BINDING_OK) { return false; }
    for (const BindingSlot *prior = sequence->first; prior != NULL; prior = prior->next) {
        if (same(prior->name, prototype.name)) {
            return binding_invalid(plan, prototype.name.site, "Duplicate name in an actual interface sequence");
        }
    }
    if (sequence->count == SIZE_MAX) { return binding_memory_failure(plan); }
    BindingSlot *slot = binding_allocate(plan, sizeof(*slot));
    if (slot == NULL) { return false; }
    *slot = prototype;
    slot->next = NULL;
    slot->sequence_ordinal = sequence->count;
    if (sequence->last == NULL) { sequence->first = slot; }
    else { sequence->last->next = slot; }
    sequence->last = slot;
    sequence->count++;
    return true;
}

/** Publish after every sequence is complete. An output pointer never observes
 * the interface whose construction is currently failing.
 * Contract: [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03).
 */
void binding_interface_publish(BindingPlan *plan, BindingInterface *view, BindingInterface **output) {
    if (plan->last_interface == NULL) { plan->first_interface = view; }
    else { plan->last_interface->next = view; }
    plan->last_interface = view;
    *output = view;
}
