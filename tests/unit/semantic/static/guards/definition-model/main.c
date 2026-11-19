/** @file
 * @brief Exhaust all three-definition relations including alias nodes and nonempty cycle witnesses.
 */
#include "../common.c"

/** A finite graph is acyclic iff one full permutation orders every dependency
 * forward. This oracle enumerates all six orders independently of reachability.
 */
static bool acyclic(unsigned graph) {
    static const size_t orders[][3] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
    for (size_t order = 0; order < 6; order++) {
        size_t position[3];
        for (size_t index = 0; index < 3; index++) { position[orders[order][index]] = index; }
        bool valid = true;
        for (size_t from = 0; from < 3; from++) {
            for (size_t to = 0; to < 3; to++) {
                if ((graph & (1u << (from * 3 + to))) != 0 && position[from] >= position[to]) { valid = false; }
            }
        }
        if (valid) { return true; }
    }
    return false;
}

/** Inputs are already scoped abstract edges for the graph kernel. Source
 * expression-to-dependency evaluation belongs to T211, not this finite oracle.
 */
static void edges(unsigned graph, StaticDefinitionNode **nodes, SourceSpan site, bool reverse) {
    for (size_t index = 0; index < 9; index++) {
        size_t bit = reverse ? 8 - index : index;
        if ((graph & (1u << bit)) == 0) { continue; }
        StaticDefinitionNode *source = nodes[bit / 3];
        StaticDefinitionEdge *edge = static_guard_allocate(&guards, sizeof(*edge));
        assert(edge);
        *edge = (StaticDefinitionEdge){.target = nodes[bit % 3], .site = site};
        if (source->last) { source->last->next = edge; }
        else { source->first = edge; }
        source->last = edge;
        guards.definition_edge_count++;
    }
}

/** All 512 directed relations, including self edges, are checked in both node/
 * edge discovery orders. Two results return the same Type but stay distinct
 * definition identities; the third node is an actual original receiver alias.
 */
static void it_matches_every_three_definition_relation_and_discovery_order(void) {
    setup_guards("@...\n...a\n..integer\n...b\n..integer\n"
        ".other\n ...out > ...alias\n", "@...\n...out\n..integer\n");
    const char *names[] = {"...a", "...b", "...alias"};
    const StaticSymbol *symbols[3];
    for (size_t index = 0; index < 3; index++) {
        symbols[index] = static_symbol_find(caller_module, static_name_site(project.entry, names[index]));
        assert(symbols[index] && (symbols[index]->role == STATIC_RESULT || symbols[index]->role == STATIC_ALIAS));
    }
    assert(symbols[2]->role == STATIC_ALIAS);
    const StaticTermEnvironment *environment;
    assert(static_term_producer_environment(&terms, caller_module, NULL, 0, &environment));
    size_t baseline = live_allocations;
    for (unsigned graph = 0; graph < 512; graph++) {
        for (size_t reverse = 0; reverse < 2; reverse++) {
            const StaticCreationPath *root;
            assert(static_creation_root(&guards, environment, &root));
            StaticDefinitionNode *nodes[3];
            for (size_t index = 0; index < 3; index++) {
                size_t key = reverse ? 2 - index : index;
                assert(static_guard_definition(&guards, root, symbols[key], &nodes[key]));
            }
            StaticGuardClosure closure = STATIC_GUARD_CLOSED;
            assert(static_guard_definition_keys(&guards, &closure) && closure == STATIC_GUARD_CLOSED);
            assert(nodes[0] != nodes[1] && nodes[0]->representative != nodes[1]->representative);
            edges(graph, nodes, symbols[0]->site, reverse != 0);
            size_t before = live_allocations;
            bool accepted = static_guard_definition_graph(&guards);
            assert(accepted == acyclic(graph));
            if (accepted) { assert(live_allocations == before); }
            else {
                assert(guards.status == STATIC_GUARD_CONFLICT && guards.definition_witness && guards.definition_closing_edge);
                const StaticDefinitionSearch *entry = guards.definition_witness;
                size_t length = 1;
                while (entry->parent) {
                    assert(entry->edge->target->representative == entry->node);
                    entry = entry->parent;
                    length++;
                }
                assert(length <= 3 && entry->node == guards.definition_closing_edge->target->representative);
                assert(diagnostic_valid(&guards.error));
            }
            static_guard_destroy(&guards);
            assert(live_allocations == baseline);
            static_guard_init(&guards, &applications, sources.allocator);
        }
    }
    destroy_guards();
}

/** Execute the independent finite directed-cycle model. */
static void run(void) { it_matches_every_three_definition_relation_and_discovery_order(); }
/** Return success after complete counted teardown. */
int main(void) { run(); return 0; }
