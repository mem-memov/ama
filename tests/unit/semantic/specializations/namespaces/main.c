/** @file
 * @brief Verify original constant namespace evidence through ordinary initializer and saved-read consumers.
 */
#include "../common.c"
#include "semantic/specializations/namespace.h"

/** Own one independently projected inventory before ordinary checking. */
static SemanticType *prepare(void) {
    assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_OK);
    assert(semantic_native(&semantics) == SEMANTIC_OK);
    SemanticType *main_type = semantics.first;
    while (main_type->module != project.entry) { main_type = main_type->next; }
    return main_type;
}

/** Both source forms retain the same exact constant declaration after checking. */
static void it_resolves_qualified_and_saved_namespaces_from_original_evidence(void) {
    setup("@!\ndirect\n .other.value\n.other\n value >> saved\n",
        "@=\nvalue\n..integer\n |7\n@!\n", true);
    SemanticType *main_type = prepare();
    assert(semantic_resolve(&semantics) == SEMANTIC_OK);
    assert(field(main_type, "direct")->type == field(main_type, "saved")->type);
    assert(main_type->statements->source->constant);
    assert(strcmp(main_type->statements->source->constant->owner->module->source->logical_path, ".other") == 0);
    assert(semantic_relations(&semantics) == SEMANTIC_OK);
    destroy();
}

/** Static exports distinguish two capture maps even though the underlying path is identical. */
static void it_resolves_multiple_specializations_of_one_constant_namespace(void) {
    directory(STATIC_SCOPE_ROOT_DIRECTORY);
    file("generated/static scope/container", "@=\nempty[]\n...slot\n@<\ninput\n...slot\n@!\n");
    setup("@!\n.other\n ...numbers\n empty[] >> xs[]\n.other\n ...texts\n empty[] >> ys[]\n",
        "@...\n...numbers\n.container\n ...slot < ..integer\n...texts\n.container\n ...slot < ..text\n", true);
    SemanticType *main_type = prepare();
    assert(!semantic_find_type(&semantics, ".container"));
    assert(semantic_resolve(&semantics) == SEMANTIC_OK);
    SemanticSymbol *xs = field(main_type, "xs"), *ys = field(main_type, "ys");
    assert(xs->list && ys->list && xs->type != ys->type);
    assert(strcmp(xs->type->module->source->logical_path, "..integer") == 0);
    assert(strcmp(ys->type->module->source->logical_path, "..text") == 0);
    SemanticStatement *first = main_type->statements, *second = first->next;
    assert(first->namespace_type != second->namespace_type);
    assert(first->namespace_type->module == second->namespace_type->module);
    assert(semantic_relations(&semantics) == SEMANTIC_OK);
    destroy();
}

/** Relation ordering keys distinguish captured owners before comparing their member ordinals. */
static void it_distinguishes_specialized_relation_keys_and_orders_owner_members(void) {
    directory(STATIC_SCOPE_ROOT_DIRECTORY);
    file("generated/static scope/container", "@=\nfirst[]\n...slot\nsecond[]\n...slot\n@<\ninput\n...slot\n@!\n");
    setup("@!\n.other\n ...numbers\n first[] >> xs[]\n.other\n ...texts\n second[] >> ys[]\n",
        "@...\n...numbers\n.container\n ...slot < ..integer\n...texts\n.container\n ...slot < ..text\n", true);
    assert(semantic_specialization_names(&semantics, &analysis.closure) == SEMANTIC_OK);
    assert(semantic_native(&semantics) == SEMANTIC_OK);
    assert(semantic_resolve(&semantics) == SEMANTIC_OK);
    assert(semantic_relations(&semantics) == SEMANTIC_OK);
    const DependencyGraph *graphs[] = {&semantics.eager, &semantics.constants};
    for (size_t index = 0; index < 2; index++) {
        for (const GraphVertex *a = graphs[index]->first; a; a = a->next) {
            for (const GraphVertex *b = a->next; b; b = b->next) {
                assert(strcmp(a->module_path, b->module_path) != 0 || a->ordinal != b->ordinal);
            }
        }
    }
    const SemanticSymbol *ordered[4] = {NULL, NULL, NULL, NULL};
    size_t count = 0;
    const ProjectModule *container = project_find(&project, ".container");
    for (const GraphVertex *vertex = semantics.constants.order; vertex; vertex = vertex->order_next) {
        const SemanticSymbol *symbol = vertex->value;
        if (symbol->owner->module != container) { continue; }
        assert(count < 4);
        ordered[count++] = symbol;
    }
    assert(count == 4);
    assert(ordered[0]->owner == ordered[1]->owner && ordered[2]->owner == ordered[3]->owner);
    assert(ordered[0]->owner != ordered[2]->owner);
    for (size_t index = 0; index < 4; index++) { assert(ordered[index]->role_ordinal == index % 2); }
    destroy();
}

/** A copied AST source is not an original occurrence with retained closure evidence. */
static void it_refuses_unregistered_source_identity_without_namespace_guessing(void) {
    setup("@!\nreceived\n .other.value\n", "@=\nvalue\n..integer\n |7\n@!\n", true);
    SemanticType *main_type = prepare();
    const AstSource *original = project.entry->syntax.body.first->as.assignment.source;
    AstSource copied = *original;
    assert(semantic_qualified_namespace(&semantics, main_type, original));
    assert(!semantic_qualified_namespace(&semantics, main_type, &copied));
    assert(semantics.status == SEMANTIC_INVALID && diagnostic_valid(&semantics.error));
    assert(!semantics.resolved);
    destroy();
}

/** Count every allocation in the actual closed-namespace and ordinary-check handoff. */
static void it_unwinds_every_namespace_semantic_allocation_refusal(void) {
    size_t total = 0;
    for (size_t cut = 0; cut <= total; cut++) {
        setup("@!\ndirect\n .other.value\n.other\n value >> saved\n",
            "@=\nvalue\n..integer\n |7\n@!\n", true);
        size_t before = allocation_count;
        if (cut) { failure_at = before + cut; }
        SemanticStatus status = semantic_specialization_names(&semantics, &analysis.closure);
        if (status == SEMANTIC_OK) { status = semantic_native(&semantics); }
        if (status == SEMANTIC_OK) { status = semantic_resolve(&semantics); }
        if (status == SEMANTIC_OK) { status = semantic_relations(&semantics); }
        if (!cut) { assert(status == SEMANTIC_OK); total = allocation_count - before; assert(total); }
        else {
            if (status != SEMANTIC_OUT_OF_MEMORY || !diagnostic_valid(&semantics.error)) {
                fprintf(stderr, "Namespace refusal position %zu/%zu: status %d, message %s\n",
                    cut, total, (int)status, semantics.error.message ? semantics.error.message : "(none)");
            }
            assert(status == SEMANTIC_OUT_OF_MEMORY && diagnostic_valid(&semantics.error));
        }
        failure_at = 0;
        destroy();
    }
    printf("closed namespace semantic allocation refusal positions: %zu\n", total);
}

/** Explicit dispatch preserves positive, capture-sensitive and refusal partitions. */
static void run(void) {
    it_resolves_qualified_and_saved_namespaces_from_original_evidence();
    it_resolves_multiple_specializations_of_one_constant_namespace();
    it_distinguishes_specialized_relation_keys_and_orders_owner_members();
    it_refuses_unregistered_source_identity_without_namespace_guessing();
    it_unwinds_every_namespace_semantic_allocation_refusal();
}
/** Run every explicit assertion case and report success after counted teardown. */
int main(void) { run(); return 0; }
