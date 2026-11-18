/** @file
 * @brief Compare exact finite solving with independent Boolean graph closure.
 */
#include "../common.c"

/** Boolean transitive closure is an independent oracle, without variable links,
 * nominal decomposition, signature checks or the solver's representative rules.
 */
static void connectivity(bool connected[5][5]) {
    for (size_t middle = 0; middle < 5; middle++) {
        for (size_t left = 0; left < 5; left++) {
            for (size_t right = 0; right < 5; right++) {
                connected[left][right] = connected[left][right] ||
                    (connected[left][middle] && connected[middle][right]);
            }
        }
    }
}

/** Enumerate all 120 distinct three-edge subsets among five vertices, and all
 * six permutations each. Three unknowns are required; two distinct known nominal
 * anchors conflict exactly when connected. Missing anchors remain Pending.
 */
static void it_matches_all_finite_three_edge_models_in_every_equation_order(void) {
    size_t edges[10][2], edge_count = 0, checked = 0;
    for (size_t left = 0; left < 5; left++) {
        for (size_t right = left + 1; right < 5; right++) {
            edges[edge_count][0] = left;
            edges[edge_count++][1] = right;
        }
    }
    assert(edge_count == 10);
    const size_t orders[6][3] = {{0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}};
    for (size_t first = 0; first < 10; first++) {
        for (size_t second = first + 1; second < 10; second++) {
            for (size_t third = second + 1; third < 10; third++) {
                size_t selected[] = {first, second, third};
                bool connected[5][5] = {{false}};
                for (size_t index = 0; index < 5; index++) { connected[index][index] = true; }
                for (size_t index = 0; index < 3; index++) {
                    size_t left = edges[selected[index]][0], right = edges[selected[index]][1];
                    connected[left][right] = connected[right][left] = true;
                }
                connectivity(connected);
                bool conflict = connected[3][4], closed = true;
                for (size_t index = 0; index < 3; index++) { closed = closed && (connected[index][3] || connected[index][4]); }
                for (size_t order = 0; order < 6; order++) {
                    setup_constraints("@<\n...a\n...b\n...c\n@!\n");
                    const StaticTerm *terms[] = {left_variable, right_variable, constraint_symbol("...c"), plain_type, NULL};
                    StaticTermCapture captures[3] = {{0}};
                    const StaticSymbol *formal = constraint_module->first;
                    for (size_t index = 0; index < 3; index++, formal = formal->next) {
                        captures[index].formal = formal;
                        captures[index].value = plain_type;
                        assert(static_constraint_require(&solver, terms[index], terms[index]->site));
                    }
                    assert(static_term_nominal(&term_store, constraint_module, captures, 3,
                        left_variable->site, &terms[4]));
                    for (size_t index = 0; index < 3; index++) {
                        size_t edge = selected[orders[order][index]];
                        assert(static_constraint_equate(&solver, terms[edges[edge][0]], terms[edges[edge][1]], left_variable->site));
                    }
                    StaticConstraintOutcome outcome = STATIC_CONSTRAINT_PENDING;
                    bool success = static_constraint_solve(&solver, &outcome);
                    assert(success == !conflict);
                    if (conflict) { assert(solver.status == STATIC_CONSTRAINT_CONFLICT); }
                    else {
                        assert(outcome == (closed ? STATIC_CONSTRAINT_COMPLETE : STATIC_CONSTRAINT_PENDING));
                        for (size_t index = 0; index < 3; index++) {
                            if (connected[index][3] || connected[index][4]) {
                                const StaticTerm *value = NULL;
                                StaticConstraintOutcome value_outcome;
                                assert(static_constraint_value(&solver, terms[index], &value, &value_outcome));
                                assert(value_outcome == STATIC_CONSTRAINT_COMPLETE);
                                assert(value == terms[connected[index][3] ? 3 : 4]);
                            }
                        }
                    }
                    destroy_constraints();
                    checked++;
                }
            }
        }
    }
    assert(checked == 720);
}

/** Run the finite domain exhaustively, not a randomized subset. */
static void run(void) { it_matches_all_finite_three_edge_models_in_every_equation_order(); }
/** Return success after every model/order and counted teardown. */
int main(void) { run(); return 0; }
