/**
 * @file
 * @brief Build required eager-owned type edges independently of execution dependencies.
 *
 * [TYPE-04](../../../spec/language/declarations.md#type-04)
 * Nonowning references and empty recursive lists do not create eager construction cycles.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) Inferred owned fields participate in the same eager-construction graph.
 */
#include "semantic/internal.h"

/** Build exactly the TYPE-04 prerequisites after initializer resolution.
 * Every emitted edge requires at least one child construction before its owner
 * completes. An empty list emits no such requirement. A finite DAG therefore
 * supplies a construction order, whereas a cycle requires infinite eager nesting.
 * Borrowed type/field sites outlive the graph; allocation failure leaves it owned.
 */
bool semantic_eager_graph(SemanticProgram *program) {
    DependencyGraph *graph = &program->eager;
    for (SemanticType *type = program->first; type != NULL; type = type->next) {
        if (type->executable && semantic_type_vertex(graph, type) == NULL) {
            return false;
        }
    }
    for (SemanticType *type = program->first; type != NULL; type = type->next) {
        if (!type->executable) {
            continue;
        }
        GraphVertex *owner = semantic_type_vertex(graph, type);
        for (SemanticSymbol *field = type->first; field != NULL; field = field->next) {
            bool eager = field->role != SYMBOL_CONSTANT && !field->reference
                && (!field->list || field->initializers != NULL);
            if (eager && !dependency_edge(graph, owner, semantic_type_vertex(graph, field->type),
                    field->type_site)) {
                return false;
            }
        }
    }
    return true;
}

GraphVertex *semantic_type_vertex(DependencyGraph *graph, SemanticType *type) {
    return dependency_vertex(graph, type, type->module->source->logical_path, 0,
        (SourceSpan){type->module->source->text, 0, 0});
}

GraphVertex *semantic_constant_vertex(DependencyGraph *graph, SemanticSymbol *symbol) {
    return dependency_vertex(graph, symbol, symbol->owner->module->source->logical_path,
        symbol->role_ordinal, symbol->spelling);
}
