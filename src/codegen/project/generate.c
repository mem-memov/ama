/** @file
 * @brief Own shared-assembly and explicitly selected application artifact construction.
 * [CLI-01](../../../spec/implementation/launch-and-embedding.md#cli-01)
 * admits actual executable roots; [EMBED-01](../../../spec/implementation/c-library.md#embed-01)
 * keeps library assembly independent of one-shot launcher configuration.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) makes readiness a
 * complete-output certificate rather than evidence of an attempted emission.
 */
#include <assert.h>
#include "codegen/internal.h"

bool project_codegen_build(NativeScaffold *scaffold, ScaffoldOptions options) {
    assert(!scaffold->attempted);
    scaffold->attempted = true;
    scaffold->ready = codegen_project_assembly(scaffold, options);
    return scaffold->ready;
}
bool project_delivery_build(NativeScaffold *scaffold, ScaffoldOptions options, const DeliveryRequest *request) {
    assert(!scaffold->attempted);
    scaffold->attempted = true;
    if (!delivery_selection_prepare(&scaffold->arena, scaffold->frontend, request, &scaffold->delivery, &scaffold->error) ||
        !delivery_inputs_prepare(&scaffold->arena, scaffold->delivery, &scaffold->launch_inputs, &scaffold->error)) { return false; }
    if (scaffold->delivery->mode != DELIVERY_EXECUTABLE) {
        return scaffold_error(scaffold, DIAGNOSTIC_UNSUPPORTED_FEATURE,
            "Source-inclusion packaging is not available in this emitter", NULL);
    }
    if (!codegen_project_assembly(scaffold, options)) { return false; }
    const char *entry = scaffold_join(scaffold, scaffold->directory, "/", "launcher.c");
    if (!scaffold_artifact(scaffold, entry, false, codegen_emit_launcher, scaffold->selected)) { return false; }
    scaffold->entry_artifact = scaffold->last;
    const char *const names[] = {"compile_commands.json", "delivery.json"};
    ScaffoldEmitter emitters[] = {codegen_emit_tooling, codegen_emit_delivery};
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        const char *path = scaffold_join(scaffold, scaffold->directory, "/", names[index]);
        if (!scaffold_artifact(scaffold, path, false, emitters[index], scaffold->selected)) { return false; }
    }
    scaffold->ready = true;
    return true;
}
