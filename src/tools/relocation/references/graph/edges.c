/** @file
 * @brief Retain every resolved module edge and select only explicit changed path spans.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original bindings through cycles and external importers;
 * [LIST-04](../../../../../spec/language/lists.md#list-04) keeps fixed implicit dependencies distinct from written names.
 */
#include <string.h>
#include "../internal.h"

/** Source-backed exact paths must agree with their resolved target before any replacement is recorded. */
static bool append(RelocationReferences *references, const RelocationModule *owner, const ProjectDependency *edge) {
    const RelocationModule *target = relocation_reference_module(references,edge->target);
    if (target == NULL || !source_span_valid(edge->requirement)) {
        return relocation_references_fail(references,RELOCATION_INVALID_REFERENCE,"A dependency has no exact captured source mapping",edge->requirement);
    }
    if (!edge->implicit && (edge->requirement.end-edge->requirement.start != strlen(edge->logical_path)
        || memcmp(source_bytes(edge->requirement.source)+edge->requirement.start,edge->logical_path,strlen(edge->logical_path)) != 0)) {
        return relocation_references_fail(references,RELOCATION_INVALID_REFERENCE,"A module path does not match its resolved source binding",edge->requirement);
    }
    bool changed = strcmp(edge->logical_path,target->after_identity) != 0;
    if (edge->implicit && changed) {
        return relocation_references_fail(references,RELOCATION_INVALID_REFERENCE,"A fixed language dependency cannot change identity",edge->requirement);
    }
    RelocationReference *item = source_arena_allocate(&references->arena,sizeof(*item));
    if (item == NULL) {
        return relocation_references_fail(references,RELOCATION_OUT_OF_MEMORY,"Reference graph allocation failed",edge->requirement);
    }
    *item = (RelocationReference){.owner=owner,.target=target,.site=edge->requirement,.implicit=edge->implicit};
    if (references->last == NULL) { references->first = item; }
    else { references->last->next = item; }
    references->last = item;
    return !changed || relocation_reference_edit(references,edge->requirement,target);
}

/** All active modules are registered before parsing, so module cycles require no rewriting order. */
bool relocation_references_graph(RelocationReferences *references) {
    for (const ProjectModule *module = references->project.first; module != NULL; module = module->next) {
        const RelocationModule *owner = relocation_reference_module(references,module);
        if (owner == NULL) {
            return relocation_references_fail(references,RELOCATION_INVALID_REFERENCE,"A module has no exact captured source mapping",(SourceSpan){0});
        }
        for (const ProjectDependency *edge = module->first; edge != NULL; edge = edge->next) {
            if (!append(references,owner,edge)) { return false; }
        }
    }
    return true;
}
