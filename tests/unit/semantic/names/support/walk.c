/** @file
 * @brief Check physical visitation order, loop-depth ownership, stable-sort bounds and complete inventory failure cleanup.
 */
#include "semantic/walk/internal.h"
/** Finite test-owned trace of actual semantic walker callback events. */
typedef struct { const AstNode *nodes[16]; size_t depths[16]; const AstNode *loops[16]; size_t count; bool stop; } WalkTrace;
/** Capture exact node identity and lexical loop depth, optionally declining the first event. */
static bool capture_walk(SemanticProgram *program, SemanticType *type,
    const AstNode *node, size_t depth, const AstNode *loop, void *context) {
    (void)program;
    (void)type;
    WalkTrace *trace = context;
    assert(trace->count < 16);
    trace->nodes[trace->count] = node;
    trace->loops[trace->count] = loop;
    trace->depths[trace->count++] = depth;
    return !trace->stop;
}
/** Header handlers keep incoming loop depth while loop and traversal bodies add their own scopes. */
static void it_visits_every_owned_subtree_in_source_order_with_exact_header_handler_depth(void) {
    directory("generated/walk scopes");
    file("generated/walk scopes/main", "@!\nouter\n{\n root !\n  child !\n   condition\n   {\n    items[item&]\n    {\n     ^\n    }\n   }\n  :\n   ^\n :\n  ^\n}\n");
    setup_semantic();
    SemanticStatus status = load_semantic("generated/walk scopes");
    assert(status == SEMANTIC_OK);
    SemanticType *type = semantic.first;
    const AstNode *outer = type->module->syntax.body.first;
    const AstNode *inner = outer->as.control.body.first;
    const AstNode *root = inner->as.control.command;
    const AstNode *child = root->as.call.first_output->operation;
    const AstNode *traversal = inner->as.control.body.first;
    const AstNode *expected[] = {outer, inner, root, child, traversal,
        traversal->as.traversal.body.first, child->as.call.alternative.first, root->as.call.alternative.first};
    const AstNode *loops[] = {NULL, outer, outer, outer, inner, traversal, outer, outer};
    const size_t depths[] = {0, 1, 1, 1, 2, 3, 1, 1};
    WalkTrace trace = {0};
    bool visited = semantic_walk(&semantic, type, type->module->syntax.body, capture_walk, &trace);
    assert(visited && trace.count == 8);
    for (size_t index = 0; index < 8; index++) {
        assert(trace.nodes[index] == expected[index] && trace.depths[index] == depths[index] && trace.loops[index] == loops[index]);
        if (index != 0) { assert(trace.nodes[index - 1]->span.start <= trace.nodes[index]->span.start); }
    }
    trace = (WalkTrace){.stop = true};
    visited = semantic_walk(&semantic, type, type->module->syntax.body, capture_walk, &trace);
    assert(!visited && trace.count == 1 && trace.nodes[0] == outer);
    destroy_semantic();
}
/** Stable sort preserves identity/cardinality/tail through empty, singleton, equal-key and power-of-two boundaries. */
static void it_preserves_visit_multisets_and_equal_site_order_across_sort_width_boundaries(void) {
    setup_semantic();
    const SourceText *source = NULL;
    SourceStatus added = source_manager_add(&sources, "visit positions", (const unsigned char *)"abcdefghijklmnopq", 17, &source);
    assert(added == SOURCE_OK);
    for (size_t count = 0; count <= 17; count++) {
        for (size_t grouped = 0; grouped < 2; grouped++) {
            AstNode nodes[17] = {0};
            SemanticWalkItem items[17];
            for (size_t index = 0; index < count; index++) {
                size_t offset = grouped ? (count - 1 - index) / 2 : count - 1 - index;
                nodes[index].span = (SourceSpan){source, offset, offset + 1};
                items[index] = (SemanticWalkItem){index + 1 == count ? NULL : &items[index + 1], &nodes[index], 0, NULL};
            }
            SemanticWalk walk = {count == 0 ? NULL : items, count == 0 ? NULL : &items[count - 1], count};
            size_t allocations = allocation_count;
            semantic_walk_sort(&walk);
            assert(allocation_count == allocations && walk.count == count);
            const SemanticWalkItem *previous = NULL;
            size_t visited = 0;
            bool seen[17] = {false};
            for (const SemanticWalkItem *item = walk.first; item != NULL; item = item->next) {
                size_t identity = (size_t)(item - items);
                assert(identity < count && !seen[identity]);
                seen[identity] = true;
                if (previous != NULL) {
                    assert(previous->node->span.start <= item->node->span.start);
                    if (previous->node->span.start == item->node->span.start) { assert(previous < item); }
                }
                if (!grouped) { assert(item->node->span.start == visited); }
                previous = item;
                visited++;
            }
            assert(visited == count && previous == walk.last);
        }
    }
    destroy_project();
}
/** Every allocation of the expanded inventory, including semantic walk storage, leaves one destructible owner on failure. */
static void it_unwinds_every_complete_inventory_allocation_failure(void) {
    prepare_complete_inventory();
    setup_semantic();
    load_complete_inventory();
    size_t total = allocation_count;
    destroy_semantic();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_semantic();
        ModuleRoot roots[] = {{"generated/inventory", NULL, 0, MODULE_ROOT_PROJECT},
            {"generated/standard inventory", NULL, 0, MODULE_ROOT_STANDARD}};
        ProjectStatus loaded = project_load(&project, roots, 2, ".main");
        assert(loaded == PROJECT_OK);
        if (failure <= allocation_count) { destroy_project(); continue; }
        semantic_program_init(&semantic, &project, sources.allocator);
        failure_at = failure;
        SemanticStatus status = semantic_names(&semantic);
        assert(status == SEMANTIC_OUT_OF_MEMORY && !semantic.names_valid);
        assert(diagnostic_valid(&semantic.error));
        destroy_semantic();
    }
}
