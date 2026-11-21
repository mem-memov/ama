/** @file
 * @brief Publish a complete owned component/mode selection after checked root admission.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) retains failed
 * scratch and borrowed IR lifetimes; [CLI-01](../../../spec/implementation/launch-and-embedding.md#cli-01)
 * forbids runtime arguments from influencing the selected root identity.
 */
#include "internal.h"
bool delivery_selection_prepare(SourceArena *arena, const CompilerFrontend *frontend, const DeliveryRequest *request,
    const DeliverySelection **output, Diagnostic *error) {
    if (!arena || !output || !error || !delivery_selection_validate(frontend, request, error)) { return false; }
    DeliverySelection *selection = source_arena_allocate(arena, sizeof(*selection));
    if (!selection) { goto no_memory; }
    char *component = source_arena_copy(arena, request->component);
    if (!component) { goto no_memory; }
    *selection = (DeliverySelection){request->mode, component, frontend, frontend->plan.entry};
    *output = selection;
    return true;
no_memory:
    *error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY, .message = "Cannot retain the complete delivery selection"};
    return false;
}
