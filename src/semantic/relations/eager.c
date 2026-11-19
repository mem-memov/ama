/**
 * @file
 * @brief Build required eager-owned type edges independently of execution dependencies.
 *
 * [TYPE-04](../../../spec/language/declarations.md#type-04)
 * Nonowning references and empty recursive lists do not create eager construction cycles.
 * [TPL-05](../../../spec/language/templates.md#tpl-05) separates distinct captured relation keys.
 * [NAME-02](../../../spec/implementation/compiler.md#name-02) provides complete checked key displays.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) Inferred owned fields participate in the same eager-construction graph.
 */
#include "semantic/internal.h"
#include "semantic/static/closure/plan.h"

/** Build exactly the TYPE-04 prerequisites after initializer resolution.
 * Every emitted edge requires at least one child construction before its owner
 * completes. An empty list emits no such requirement. A finite DAG therefore
 * supplies a construction order, whereas a cycle requires infinite eager nesting.
 * Borrowed type/field sites outlive the graph; allocation failure leaves it owned.
 * Component contract: [TYPE-04](../../../spec/language/declarations.md#type-04), [TYPE-06](../../../spec/language/declarations.md#type-06). */
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

/** Use the already checked complete specialization display as a stable graph
 * key. Its fixed descriptor framing distinguishes captures; zero-input ordinary
 * identities retain their original path. The naming owner outlives semantic
 * graphs, so no allocation or digest-based tie-break is needed.
 * [TPL-05](../../../spec/language/templates.md#tpl-05) distinguishes captured owners.
 * [NAME-02](../../../spec/implementation/compiler.md#name-02) retains full keys.
 */
static const char *relation_name(const SemanticType *type) {
    return type->specialization
        ? type->specialization->context->registry_use->candidate->name->source_name
        : type->module->source->logical_path;
}

GraphVertex *semantic_type_vertex(DependencyGraph *graph, SemanticType *type) {
    return dependency_vertex(graph, type, relation_name(type), 0,
        (SourceSpan){type->module->source->text, 0, 0});
}

GraphVertex *semantic_constant_vertex(DependencyGraph *graph, SemanticSymbol *symbol) {
    return dependency_vertex(graph, symbol, relation_name(symbol->owner),
        symbol->role_ordinal, symbol->spelling);
}
