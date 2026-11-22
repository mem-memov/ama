/** @file
 * @brief Project checked source definitions into plain transportable delivery records.
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03) retains original edge
 * causes; [PKG-08](../../../spec/project/packages.md#pkg-08) selects concrete
 * executable tests separately from open/static/constant support.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01) prevents
 * project consumers from depending on live compiler owners.
 * [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02)
 * keeps fixed dependency sites and normalized constant members explicit.
 */
#include "export.h"
#include <string.h>
#include <stdint.h>
/** Preserve every resolved edge and original span without reinterpreting scopes
 * or dropping compile-time dependencies under
 * [DEP-03](../../../spec/implementation/compiler.md#dep-03).
 * Keep implicit sites distinct and copy normalized constant members under
 * [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02);
 * original operand text may not contain its normalized components contiguously.
 */
static SourceStatus dependencies(SourceArena *arena, const ProjectModule *module,
    DeliverySourceDependency **output) {
    DeliverySourceDependency *first = NULL, **tail = &first;
    for (const ProjectDependency *edge = module->first; edge; edge = edge->next) {
        DeliverySourceDependency *copy = source_arena_allocate(arena, sizeof(*copy));
        const char *target = copy ? source_arena_copy(arena, edge->target->source->logical_path) : NULL;
        if (!target) { return SOURCE_OUT_OF_MEMORY; }
        const char *member = NULL;
        if (edge->constant_path) {
            ModulePathPart part = edge->constant_path->member;
            if (part.length == SIZE_MAX) { return SOURCE_OUT_OF_MEMORY; }
            char *name = source_arena_allocate(arena, part.length + 1);
            if (!name) { return SOURCE_OUT_OF_MEMORY; }
            memcpy(name, part.bytes, part.length); name[part.length] = 0;
            member = name;
        }
        *copy = (DeliverySourceDependency){.target = target, .origin = edge->requirement,
            .implicit = edge->implicit, .member = member};
        *tail = copy; tail = &copy->next;
    }
    *output = first; return SOURCE_OK;
}
SourceStatus delivery_source_export(SourceArena *arena, const StaticInventory *inventory,
    const DeliverySourceDefinition **output) {
    if (!arena || !inventory || !inventory->valid || !inventory->project || !inventory->project->valid || !output) {
        return SOURCE_INVALID_ARGUMENT;
    }
    DeliverySourceDefinition *first = NULL;
    for (const StaticModule *module = inventory->first; module; module = module->next) {
        DeliverySourceDefinition *copy = source_arena_allocate(arena, sizeof(*copy));
        const char *identity = copy ? source_arena_copy(arena, module->definition->source->logical_path) : NULL;
        if (!identity) { return SOURCE_OUT_OF_MEMORY; }
        ParsedModuleKind kind = module->definition->syntax.sections.kind;
        DeliverySourceKind category = kind == MODULE_STATIC || kind == MODULE_MARKER ? DELIVERY_SOURCE_STATIC
            : kind == MODULE_CONSTANTS ? DELIVERY_SOURCE_CONSTANTS
            : module->parameter_count ? DELIVERY_SOURCE_PARAMETERIZED : DELIVERY_SOURCE_CONCRETE;
        *copy = (DeliverySourceDefinition){.identity = identity, .kind = category};
        SourceStatus status = dependencies(arena, module->definition, &copy->dependencies);
        if (status != SOURCE_OK) { return status; }
        DeliverySourceDefinition **position = &first;
        while (*position && strcmp((*position)->identity, identity) < 0) { position = &(*position)->next; }
        copy->next = *position; *position = copy;
    }
    *output = first; return SOURCE_OK;
}
