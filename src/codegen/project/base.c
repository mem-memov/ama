/** @file
 * @brief Prepare shared project declarations/body without creating application launcher artifacts.
 * [BUILD-01](../../../spec/implementation/application-builds.md#build-01) captures
 * the exact planned native origins before artifact construction.
 * [EMBED-01](../../../spec/implementation/c-library.md#embed-01) keeps stateful
 * library packaging separate; [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * publishes no partial artifact set after failure.
 */
#include "codegen/internal.h"
bool codegen_project_assembly(NativeScaffold *scaffold, ScaffoldOptions options) {
    scaffold->options = options;
    if (scaffold->frontend->authoring || !scaffold_paths(scaffold) || !scaffold_model(scaffold) ||
        !scaffold_emit_declarations(scaffold)) { return false; }
    const DeliveryNativeOrigin *origins = NULL;
    if (!delivery_native_origins(&scaffold->arena, scaffold->frontend, &origins, &scaffold->error)
        || !delivery_native_capture(&scaffold->arena, &scaffold->texts, origins,
            &scaffold->native_inputs, &scaffold->error)
        || !delivery_version_guidance(&scaffold->arena, &scaffold->texts, scaffold->frontend,
            scaffold->native_inputs, &scaffold->guidance, &scaffold->error)) { return false; }
    const char *const names[] = {"project.h", "project.c"};
    ScaffoldEmitter emitters[] = {codegen_emit_header, codegen_emit_body};
    for (size_t index = 0; index < 2; index++) {
        const char *path = scaffold_join(scaffold, scaffold->directory, "/", names[index]);
        if (!scaffold_artifact(scaffold, path, false, emitters[index], scaffold->selected)) { return false; }
    }
    return true;
}
