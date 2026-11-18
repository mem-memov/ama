/** @file
 * @brief Exhaust all four-vertex concrete graphs without collapsing unequal specialization keys.
 */
#include "../common.c"

/** Independent transitive closure starts with edges, not reflexive identity.
 * It therefore represents exactly nonempty paths, including directed cycles.
 */
static bool forbidden(unsigned graph) {
    bool paths[4][4];
    const unsigned labels[] = {0, 0, 1, 2};
    for (size_t from = 0; from < 4; from++) {
        for (size_t to = 0; to < 4; to++) { paths[from][to] = (graph & (1u << (from * 4 + to))) != 0; }
    }
    for (size_t middle = 0; middle < 4; middle++) {
        for (size_t from = 0; from < 4; from++) {
            for (size_t to = 0; to < 4; to++) { paths[from][to] |= paths[from][middle] && paths[middle][to]; }
        }
    }
    for (size_t from = 0; from < 3; from++) {
        for (size_t to = 0; to < 3; to++) {
            if (from != to && labels[from] == labels[to] && paths[from][to]) { return true; }
        }
    }
    return false;
}

/** The abstract kernel fixture supplies already validated finite edge metadata,
 * not a claim that arbitrary graphs are complete accepted Ama source programs.
 * One original source span supplies retained diagnostic lifetime. Source/path
 * validation is independently covered by ancestry/classification compositions.
 */
static void graph_edges(unsigned graph, StaticCreationVertex **vertices, const StaticCreationPath *trace, bool reverse) {
    for (size_t index = 0; index < 16; index++) {
        size_t bit = reverse ? 15 - index : index;
        if ((graph & (1u << bit)) == 0) { continue; }
        size_t from = bit / 4, to = bit % 4;
        StaticCreationEdge *edge = static_guard_allocate(&guards, sizeof(*edge));
        assert(edge);
        *edge = (StaticCreationEdge){.target = vertices[to], .path = trace};
        if (vertices[from]->last) { vertices[from]->last->next = edge; }
        else { vertices[from]->first = edge; }
        vertices[from]->last = edge;
        guards.edge_count++;
    }
}

/** Full source identity repeats at the witness endpoints; every intermediate
 * edge is the original retained graph edge, not an invented source-name hop.
 */
static void witness(void) {
    assert(guards.creation_witness && guards.closing_edge);
    const StaticCreationSearch *item = guards.creation_witness;
    assert(guards.closing_edge->path && guards.closing_edge->path->site.source);
    size_t length = 1;
    while (item->parent) {
        assert(item->edge && item->edge->target == item->vertex);
        item = item->parent;
        length++;
    }
    assert(length <= 4);
    assert(item->vertex->value->as.nominal.origin == guards.closing_edge->target->value->as.nominal.origin);
    assert(item->vertex != guards.closing_edge->target);
    assert(guards.earlier_creation_value == item->vertex->value);
    assert(guards.rejected_creation_value == guards.closing_edge->target->value);
    assert(diagnostic_valid(&guards.error));
}

/** Exhaust 65536 directed graphs including self edges, in both insertion orders.
 * A0/A1 share one original template but have different complete keys; B0 is
 * another template and integer is ordinary. Check cold scratch release and
 * rejected witness teardown after every graph against a fixed live baseline.
 */
static void it_matches_all_four_vertex_graphs_and_insertion_orders(void) {
    setup_guards(NULL, NULL);
    StaticTermCapture map[] = {{.formal = application_formal(target_module, 0), .value = integer_type},
        {.formal = application_formal(target_module, 1), .value = integer_type}};
    const StaticTerm *values[4];
    assert(static_term_nominal(&terms, target_module, map, 2, (SourceSpan){0}, &values[2]));
    map[0].formal = application_formal(caller_module, 0);
    map[1].formal = application_formal(caller_module, 1);
    assert(static_term_nominal(&terms, caller_module, map, 2, (SourceSpan){0}, &values[0]));
    map[0].value = values[2];
    assert(static_term_nominal(&terms, caller_module, map, 2, (SourceSpan){0}, &values[1]));
    values[3] = integer_type;
    for (size_t index = 0; index < 4; index++) {
        const StaticTerm *closed;
        StaticConstraintOutcome outcome;
        assert(static_constraint_value(&solver, values[index], &closed, &outcome));
        assert(outcome == STATIC_CONSTRAINT_COMPLETE && closed == values[index]);
    }
    StaticCreationPath trace = {.site = static_name_site(project.entry, "...a")};
    size_t baseline = live_allocations;
    for (unsigned graph = 0; graph < 65536; graph++) {
        bool expected = forbidden(graph);
        for (size_t reverse = 0; reverse < 2; reverse++) {
            StaticCreationVertex *vertices[4];
            for (size_t index = 0; index < 4; index++) {
                size_t key = reverse ? 3 - index : index;
                assert(static_guard_vertex(&guards, values[key], &vertices[key]));
            }
            graph_edges(graph, vertices, &trace, reverse != 0);
            size_t before = live_allocations;
            bool accepted = static_guard_creation_graph(&guards);
            assert(accepted == !expected);
            if (accepted) { assert(live_allocations == before && guards.status == STATIC_GUARD_OK); }
            else { assert(guards.status == STATIC_GUARD_CONFLICT); witness(); }
            static_guard_destroy(&guards);
            assert(live_allocations == baseline && applications.status == STATIC_APPLY_OK);
            static_guard_init(&guards, &applications, sources.allocator);
        }
    }
    destroy_guards();
}

/** Execute the exact finite graph domain and independent matrix oracle. */
static void run(void) { it_matches_all_four_vertex_graphs_and_insertion_orders(); }
/** Return success after counted exhaustive-domain teardown. */
int main(void) { run(); return 0; }
