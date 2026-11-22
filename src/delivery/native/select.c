/** @file
 * @brief Build the exact native-origin projection of a concrete program plan.
 * [BUILD-01](../../../spec/implementation/application-builds.md#build-01) selects
 * actual concrete dependencies, rather than the larger loaded source catalog;
 * [COMP-05](../../../spec/implementation/compiler.md#comp-05) orders complete
 * logical identities independently of discovery or runtime instance counts.
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) retains partial
 * scratch without publishing an incomplete projection.
 */
#include <string.h>
#include "closure.h"

/** Preserve the caller's output on every refusal under
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03); no failure
 * publishes the partial ordered table as a complete closure.
 */
static bool refuse(Diagnostic *error, DiagnosticCode code, const char *message) {
    if (error != NULL) { *error = (Diagnostic){.code = code, .message = message}; }
    return false;
}

bool delivery_native_origins(SourceArena *arena, const CompilerFrontend *frontend,
    const DeliveryNativeOrigin **output, Diagnostic *error) {
    if (arena == NULL || frontend == NULL || output == NULL || !frontend->valid
        || frontend->authoring || !frontend->plan_initialized) {
        return refuse(error, DIAGNOSTIC_CHANGED_INPUT, "Native build closure requires a checked executable plan");
    }
    DeliveryNativeOrigin *first = NULL;
    for (const PlannedType *plan = frontend->plan.first; plan != NULL; plan = plan->next) {
        const ModuleSource *source = plan->type->module->source;
        if (plan->type->module->syntax.sections.kind != MODULE_NATIVE) { continue; }
        DeliveryNativeOrigin **position = &first;
        while (*position != NULL && strcmp((*position)->source->logical_path, source->logical_path) < 0) {
            position = &(*position)->next;
        }
        if (*position != NULL && strcmp((*position)->source->logical_path, source->logical_path) == 0) {
            if ((*position)->source != source) {
                return refuse(error, DIAGNOSTIC_CHANGED_INPUT, "Native module identity has inconsistent source owners");
            }
            continue;
        }
        DeliveryNativeOrigin *origin = source_arena_allocate(arena, sizeof(*origin));
        if (origin == NULL) {
            return refuse(error, DIAGNOSTIC_OUT_OF_MEMORY, "Native build closure allocation failed");
        }
        *origin = (DeliveryNativeOrigin){.source = source, .next = *position};
        *position = origin;
    }
    *output = first;
    return true;
}
