/**
 * @file
 * @brief Compose in-memory native authoring artifacts without implicit filesystem replacement.
 *
 * [CABI-06](../../../spec/implementation/native-interface.md#cabi-06)
 * Only explicit publication writes completed artifacts; create/overwrite policy controls authored companions.
 */
#include <assert.h>
#include "internal.h"

/** Join one generated artifact name to the explicit output directory, then stage complete bytes under generated rather than authored ownership.
 * Component contract: [CABI-06](../../../spec/implementation/native-interface.md#cabi-06). */
static bool generated(NativeScaffold *scaffold, const char *name, ScaffoldEmitter emitter, const ScaffoldType *type) {
    const char *path = scaffold_join(scaffold, scaffold->directory, "/", name);
    return scaffold_artifact(scaffold, path, false, emitter, type);
}

bool scaffold_emit_declarations(NativeScaffold *scaffold) {
    if (!generated(scaffold, "ama-native-types.h", scaffold_emit_types, NULL)) {
        return false;
    }
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE
            && (!generated(scaffold, type->begin_name, scaffold_emit_context, type)
                || !generated(scaffold, type->end_name, scaffold_emit_end, type)
                || !generated(scaffold, type->interface_name, scaffold_emit_interface, type))) {
            return false;
        }
    }
    return generated(scaffold, "native-model.json", scaffold_emit_model, NULL);
}

bool scaffold_emit_all(NativeScaffold *scaffold) {
    if (!scaffold_emit_declarations(scaffold)) { return false; }
    const ScaffoldType *type = scaffold->selected;
    if (!generated(scaffold, type->template_name, scaffold_emit_body, type)
        || !generated(scaffold, type->test_name, scaffold_emit_test, type)) {
        return false;
    }
    if (scaffold->options.create) {
        const ModuleSource *source = type->plan->type->module->source;
        bool overwrite = scaffold->options.overwrite;
        if ((source->native_header == NULL || overwrite)
            && !scaffold_artifact(scaffold, type->header_path, true, scaffold_emit_payload, type)) {
            return false;
        }
        if ((source->native_c == NULL || overwrite)
            && !scaffold_artifact(scaffold, type->body_path, true, scaffold_emit_body, type)) {
            return false;
        }
        if ((source->native_abi == NULL || overwrite)
            && !scaffold_artifact(scaffold, scaffold_join(scaffold, type->ama_path, "", ".abi"),
                true, scaffold_emit_abi, type)) {
            return false;
        }
    }
    return generated(scaffold, "compile_commands.json", scaffold_emit_tooling, type);
}

bool native_scaffold_build(NativeScaffold *scaffold, ScaffoldOptions options) {
    assert(!scaffold->attempted);
    scaffold->attempted = true;
    scaffold->options = options;
    if (!scaffold_paths(scaffold) || !scaffold_model(scaffold)
        || scaffold->selected->plan->type->module->syntax.sections.kind != MODULE_NATIVE
        || !scaffold_emit_all(scaffold)) {
        return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH,
            "Native scaffolding requires a validated @C entry and complete output model", NULL);
    }
    scaffold->ready = true;
    return true;
}
