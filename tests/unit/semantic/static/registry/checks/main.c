/** @file
 * @brief Compare real registry group completion with exhaustive finite reachability expectations.
 */
#include "../common.c"

/** Three complete actual owner maps include independent equal-origin siblings;
 * validation dependencies do not manufacture constructor-creation edges.
 */
static void check_paths(const StaticCreationPath **paths) {
    const StaticModule *text_module = static_module_find(&inventory, project_find(&project, "..text"));
    const StaticTerm *text;
    assert(static_term_nominal(&terms, text_module, NULL, 0, (SourceSpan){0}, &text));
    for (size_t index = 0; index < 3; index++) {
        const StaticModule *module = index == 2 ? target_module : caller_module;
        StaticTermCapture input = {.formal = application_formal(module, 0), .value = index == 1 ? text : integer_type};
        assert(static_creation_root(&guards, key_ordinary(module, &input, 1), &paths[index]));
    }
}

/** Reset only the two registry-owned arenas, retaining actual source/guard paths. */
static void reset_registry(void) {
    static_registry_destroy(&registry);
    identity_registry_destroy(&registry_names);
    identity_registry_init(&registry_names, sources.allocator);
    static_registry_init(&registry, &guards, &registry_names, sources.allocator);
}

/** Exhaust all 512 directed three-vertex graphs, eight local-completion subsets
 * and both insertion orders. Independent reflexive Floyd reachability determines
 * whether any reachable vertex lacks a local certificate: 8192 compositions.
 */
static void it_closes_exactly_all_locally_complete_reachable_groups(void) {
    setup_registry("@<\nvalue\n...type\n@!\nanchor&\n..text\nnumber&\n..integer\n", "@<\nvalue\n...type\n@!\n");
    const StaticCreationPath *paths[3];
    check_paths(paths);
    for (unsigned graph = 0; graph < 512; graph++) {
        bool reach[3][3];
        for (unsigned from = 0; from < 3; from++) {
            for (unsigned to = 0; to < 3; to++) {
                reach[from][to] = from == to || (graph & (1U << (from * 3 + to)));
            }
        }
        for (unsigned middle = 0; middle < 3; middle++) {
            for (unsigned from = 0; from < 3; from++) {
                for (unsigned to = 0; to < 3; to++) {
                    reach[from][to] = reach[from][to] || (reach[from][middle] && reach[middle][to]);
                }
            }
        }
        for (unsigned complete = 0; complete < 8; complete++) {
            for (unsigned reversed = 0; reversed < 2; reversed++) {
                reset_registry();
                StaticRegistryUse *uses[3];
                StaticRegistryKeyState keys;
                for (unsigned index = 0; index < 3; index++) {
                    unsigned actual = reversed ? 2 - index : index;
                    assert(static_registry_use(&registry, paths[actual], &uses[actual], &keys));
                    assert(keys == STATIC_REGISTRY_KEY_CLOSED);
                }
                for (unsigned index = 0; index < 9; index++) {
                    unsigned bit = reversed ? 8 - index : index;
                    if (graph & (1U << bit)) {
                        assert(static_registry_depend(&registry, uses[bit / 3], uses[bit % 3], paths[bit / 3]->site));
                    }
                }
                for (unsigned index = 0; index < 3; index++) {
                    if (complete & (1U << index)) { assert(static_registry_complete_local(&registry, uses[index])); }
                }
                StaticRegistryCheckState closure;
                assert(static_registry_refresh_checks(&registry, &closure));
                for (unsigned from = 0; from < 3; from++) {
                    bool checked = true;
                    for (unsigned to = 0; to < 3; to++) {
                        if (reach[from][to] && !(complete & (1U << to))) { checked = false; }
                    }
                    assert(uses[from]->candidate->state == (checked ? STATIC_CANDIDATE_CHECKED : STATIC_CANDIDATE_PENDING));
                }
                assert(closure == (complete == 7 ? STATIC_REGISTRY_CHECKS_CLOSED : STATIC_REGISTRY_CHECKS_PENDING));
                assert(guards.edge_count == 0);
            }
        }
    }
    destroy_registry();
}

/** A late unknown-key dependency reopens an already checked owner. Independent
 * equations close the key, but its local checks are still required separately.
 */
static void it_reopens_warm_checks_until_late_target_keys_and_local_checks_close(void) {
    setup_registry("@<\nvalue\n...type\n@!\nanchor&\n..integer\n", "@!\n");
    const StaticCreationPath *closed, *pending;
    assert(static_creation_root(&guards, key_ordinary(target_module, NULL, 0), &closed));
    StaticRegistryUse *owner, *target;
    StaticRegistryKeyState keys;
    assert(static_registry_use(&registry, closed, &owner, &keys));
    assert(static_registry_complete_local(&registry, owner));
    StaticRegistryCheckState closure;
    assert(static_registry_refresh_checks(&registry, &closure) && closure == STATIC_REGISTRY_CHECKS_CLOSED);
    assert(static_creation_root(&guards, caller_environment, &pending));
    assert(static_registry_use(&registry, pending, &target, &keys) && keys == STATIC_REGISTRY_KEY_PENDING);
    assert(static_registry_depend(&registry, owner, target, closed->site));
    assert(owner->candidate->state == STATIC_CANDIDATE_PENDING);
    assert(static_registry_refresh_checks(&registry, &closure) && closure == STATIC_REGISTRY_CHECKS_PENDING);
    const StaticTerm *parameter;
    const StaticSymbol *formal = application_formal(caller_module, 0);
    assert(static_term_symbol(&terms, caller_environment, formal, &parameter));
    assert(static_constraint_equate(&solver, parameter, integer_type, formal->site));
    assert(static_registry_refresh_checks(&registry, &closure) && closure == STATIC_REGISTRY_CHECKS_PENDING);
    assert(target->candidate && target->candidate->state == STATIC_CANDIDATE_PENDING);
    assert(static_registry_complete_local(&registry, target));
    assert(static_registry_refresh_checks(&registry, &closure) && closure == STATIC_REGISTRY_CHECKS_CLOSED);
    size_t before = allocation_count;
    assert(static_registry_depend(&registry, owner, target, closed->site));
    assert(allocation_count == before && registry.dependency_count == 1);
    destroy_registry();
}

/** A refused dependency allocation invalidates certificates and retains owned
 * records. Invalid completion cannot turn an unknown key into a checked one.
 */
static void it_keeps_failed_checks_sticky_and_releases_partial_dependencies(void) {
    for (unsigned partition = 0; partition < 9; partition++) {
        setup_registry("@<\nvalue\n...type\n@!\nanchor&\n..integer\n", "@!\n");
        const StaticCreationPath *closed, *pending;
        assert(static_creation_root(&guards, key_ordinary(target_module, NULL, 0), &closed));
        assert(static_creation_root(&guards, caller_environment, &pending));
        StaticRegistryUse *owner, *target;
        StaticRegistryKeyState keys;
        assert(static_registry_use(&registry, closed, &owner, &keys));
        assert(static_registry_use(&registry, pending, &target, &keys));
        assert(static_registry_complete_local(&registry, owner));
        StaticRegistryUse copied = *owner;
        StaticRegistryCheckState output = STATIC_REGISTRY_CHECKS_CLOSED;
        if (partition == 0) {
            failure_at = allocation_count + 1;
            assert(!static_registry_depend(&registry, owner, target, closed->site));
            failure_at = 0;
            assert(registry.status == STATIC_REGISTRY_OUT_OF_MEMORY && registry.dependency_count == 0);
        } else if (partition == 1) {
            registry.dependency_count = SIZE_MAX;
            assert(!static_registry_depend(&registry, owner, target, closed->site));
            assert(registry.status == STATIC_REGISTRY_OUT_OF_MEMORY);
        } else if (partition == 2) { assert(!static_registry_complete_local(&registry, target)); }
        else if (partition == 3) { assert(!static_registry_complete_local(&registry, &copied)); }
        else if (partition == 4) { assert(!static_registry_depend(&registry, owner, &copied, closed->site)); }
        else if (partition == 5) { assert(!static_registry_refresh_checks(&registry, NULL)); }
        else {
            bool claim = true;
            assert(!static_registry_begin_local(&registry, partition == 6 ? &copied : owner,
                partition == 7 ? NULL : &claim, partition == 8 ? NULL : &keys));
            assert(claim);
        }
        assert(owner->candidate->state == STATIC_CANDIDATE_FAILED);
        assert(!static_registry_refresh_checks(&registry, &output) && output == STATIC_REGISTRY_CHECKS_CLOSED);
        assert(diagnostic_valid(&registry.error));
        destroy_registry();
    }
}

/** A closed canonical candidate can start once, including after a graph
 * refresh already marked it Pending. Active reuse is never completion evidence.
 */
static void it_claims_local_expansion_once_without_accepting_an_active_candidate(void) {
    setup_registry("@<\nvalue\n...type\n@!\nanchor&\n..integer\n", "@!\n");
    const StaticCreationPath *path;
    assert(static_creation_root(&guards, caller_environment, &path));
    StaticRegistryUse *use;
    StaticRegistryKeyState keys;
    assert(static_registry_use(&registry, path, &use, &keys));
    bool claim = true;
    assert(static_registry_begin_local(&registry, use, &claim, &keys));
    assert(!claim && keys == STATIC_REGISTRY_KEY_PENDING && use->candidate == NULL);
    const StaticSymbol *formal = application_formal(caller_module, 0);
    const StaticTerm *variable;
    assert(static_term_symbol(&terms, caller_environment, formal, &variable));
    assert(static_constraint_equate(&solver, variable, integer_type, formal->site));
    StaticRegistryCheckState closure;
    assert(static_registry_refresh_checks(&registry, &closure) && closure == STATIC_REGISTRY_CHECKS_PENDING);
    assert(static_registry_begin_local(&registry, use, &claim, &keys) && claim);
    assert(use->candidate->state == STATIC_CANDIDATE_PENDING && !use->candidate->local_complete);
    StaticTermCapture input = {.formal = formal, .value = integer_type};
    const StaticCreationPath *other_path;
    assert(static_creation_root(&guards, key_ordinary(caller_module, &input, 1), &other_path));
    StaticRegistryUse *other;
    assert(static_registry_use(&registry, other_path, &other, &keys));
    assert(other != use && other->candidate == use->candidate);
    assert(static_registry_begin_local(&registry, other, &claim, &keys) && !claim);
    assert(static_registry_refresh_checks(&registry, &closure) && closure == STATIC_REGISTRY_CHECKS_PENDING);
    assert(static_registry_complete_local(&registry, use));
    assert(static_registry_refresh_checks(&registry, &closure) && closure == STATIC_REGISTRY_CHECKS_CLOSED);
    assert(static_registry_begin_local(&registry, other, &claim, &keys) && !claim);
    destroy_registry();
}

/** Run exhaustive closure and a real incremental unknown-key transition. */
static void run(void) {
    it_claims_local_expansion_once_without_accepting_an_active_candidate();
    it_keeps_failed_checks_sticky_and_releases_partial_dependencies();
    it_closes_exactly_all_locally_complete_reachable_groups();
    it_reopens_warm_checks_until_late_target_keys_and_local_checks_close();
}
/** Return after counted ownership teardown. */
int main(void) { run(); return 0; }
