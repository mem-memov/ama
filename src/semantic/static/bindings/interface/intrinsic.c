/**
 * @file
 * @brief Snapshot intrinsic list interfaces from canonical descriptors without fabricating source declarations.
 *
 * [LIST-04](../../../../../spec/language/lists.md#list-04) owns method/field
 * definitions. [CALL-06](../../../../../spec/language/calls-and-results.md#call-06)
 * keeps independent runtime direction positions; intrinsic methods have no
 * owner static formals/results to insert into those sequences.
 */
#include <assert.h>
#include <string.h>
#include "../internal.h"

/** Reuse descriptor identity and static name lifetime. No table of intrinsic
 * names, field arities or types is duplicated in binding normalization.
 * Contract: [LIST-04](../../../../../spec/language/lists.md#list-04).
 */
static bool slots(BindingPlan *plan, BindingInterface *view, bool result) {
    BindingSequence *sequence = &view->sequences[result ? BINDING_RUNTIME_OUTPUT : BINDING_RUNTIME_INPUT];
    for (size_t ordinal = 0;; ordinal++) {
        const SemanticListField *field = semantic_list_position(view->list_method, result, ordinal);
        if (field == NULL) { return true; }
        BindingSlot prototype = {.name = {.bytes = (const unsigned char *)field->name, .length = strlen(field->name)},
            .original_ordinal = ordinal, .intrinsic_field = field, .reference = field->element_reference};
        if (!binding_slot_append(plan, sequence, prototype)) { return false; }
    }
}

/** Domain validation precedes allocation; all fields come from one exact
 * canonical method identity and retain their original input/result ordinals.
 * Contract: [LIST-04](../../../../../spec/language/lists.md#list-04).
 */
bool binding_list_interface(BindingPlan *plan, SemanticListMethod method, BindingInterface **output) {
    if (plan->status != BINDING_OK) { return false; }
    assert(output != NULL);
    if (semantic_list_method_name(method) == NULL) {
        return binding_invalid(plan, (SourceSpan){0}, "Invalid intrinsic list method identity");
    }
    BindingInterface *view = binding_allocate(plan, sizeof(*view));
    if (view == NULL) { return false; }
    view->kind = BINDING_LIST_INTERFACE;
    view->list_method = method;
    if (!slots(plan, view, false) || !slots(plan, view, true)) { return false; }
    binding_interface_publish(plan, view, output);
    return true;
}
