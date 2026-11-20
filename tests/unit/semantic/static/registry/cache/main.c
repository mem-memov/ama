/** @file
 * @brief Check cached descendants and same-key-cycle branches through actual producer creation paths.
 */
#include "common.c"

/** Register every existing actual guard path and its locally completed fixture
 * phase. Source inputs are explicit fixture IR, not evaluated static outputs.
 */
static void check_existing_paths(void) {
    for (const StaticCreationPath *path = guards.first_path; path; path = path->next) {
        StaticRegistryUse *use;
        StaticRegistryKeyState state;
        assert(static_registry_use(&registry, path, &use, &state) && state == STATIC_REGISTRY_KEY_CLOSED);
        assert(static_registry_complete_local(&registry, use));
    }
    StaticRegistryCheckState closure;
    assert(static_registry_refresh_checks(&registry, &closure) && closure == STATIC_REGISTRY_CHECKS_CLOSED);
}

/** A rejected new creation invalidates all previous registry certificates while
 * preserving exact guard diagnostic provenance and the caller's output receiver.
 */
static void assert_registry_propagates_creation_failure(void) {
    StaticRegistryCheckState output = STATIC_REGISTRY_CHECKS_CLOSED;
    assert(!static_registry_refresh_checks(&registry, &output));
    assert(output == STATIC_REGISTRY_CHECKS_CLOSED && registry.status == STATIC_REGISTRY_CONFLICT);
    assert(registry.error.message == guards.error.message && guards.creation_witness);
    for (const StaticRegistryCandidate *item = registry.first_candidate; item; item = item->next) {
        assert(item->state == STATIC_CANDIDATE_FAILED);
    }
}

/** New Leaf(text)->cached A(text) must inspect A's existing Leaf(integer)
 * descendant even after its candidate identity and local certificate are cached.
 */
static void it_rechecks_warm_cached_grandchildren_in_both_root_orders(void) {
    for (unsigned first = 0; first < 2; first++) {
        const StaticTerm *text = setup_producers();
        const StaticModule *leaf = static_module_find(&inventory, project_find(&project, "/test/fixture/v1.0.0/src/leaf"));
        const StaticCreationPath *root = first ? producer_root(leaf, text) : NULL;
        const StaticCreationPath *b = producer_root(target_module, text);
        const StaticCreationPath *a = producer_child(b, ".main result", caller_module, text);
        (void)producer_child(a, ".leaf result", leaf, integer_type);
        if (!first) { root = producer_root(leaf, text); }
        check_existing_paths();
        StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = text};
        const StaticCreationPath *output = NULL;
        StaticCreationOutcome outcome = STATIC_CREATION_PENDING;
        assert(!static_creation_producer_enter(&guards, root, projection(leaf, ".main result"),
            &input, 1, &output, &outcome));
        assert(output == NULL && outcome == STATIC_CREATION_PENDING);
        assert_registry_propagates_creation_failure();
        destroy_registry();
    }
}

/** Closing A(integer)->B(integer)->A(integer) never hides the subsequently
 * discovered A(integer)->B(text) branch through the already cached cycle.
 */
static void it_checks_new_outgoing_branches_of_a_checked_same_key_cycle(void) {
    const StaticTerm *text = setup_producers();
    const StaticCreationPath *a = producer_root(caller_module, integer_type);
    const StaticCreationPath *b = producer_child(a, ".other result", target_module, integer_type);
    const StaticCreationPath *again = producer_child(b, ".main result", caller_module, integer_type);
    assert(a != again && a->vertex == again->vertex);
    check_existing_paths();
    assert(registry.candidate_count == 2 && registry.use_count == 3);
    StaticTermCapture input = {.formal = application_formal(target_module, 0), .value = text};
    const StaticCreationPath *output = NULL;
    StaticCreationOutcome outcome = STATIC_CREATION_PENDING;
    assert(!static_creation_producer_enter(&guards, a, projection(caller_module, ".fixed other result"),
        &input, 1, &output, &outcome));
    assert(output == NULL && outcome == STATIC_CREATION_PENDING);
    assert_registry_propagates_creation_failure();
    destroy_registry();
}

/** Querying only the deepest use must retain its complete published ancestry
 * and validation dependencies. A fresh unrelated demanded root is also retained
 * by closure and remains pending until its local checks explicitly pass.
 */
static void it_retains_guard_demanded_paths_and_ancestors_without_caller_enumeration(void) {
    const StaticTerm *text = setup_producers();
    const StaticModule *leaf = static_module_find(&inventory, project_find(&project, "/test/fixture/v1.0.0/src/leaf"));
    const StaticCreationPath *a = producer_root(caller_module, integer_type);
    const StaticCreationPath *b = producer_child(a, ".other result", target_module, integer_type);
    const StaticCreationPath *tail = producer_child(b, ".leaf result", leaf, integer_type);
    StaticRegistryUse *use;
    StaticRegistryKeyState state;
    assert(static_registry_use(&registry, tail, &use, &state) && state == STATIC_REGISTRY_KEY_CLOSED);
    assert(registry.use_count == 3 && registry.candidate_count == 3 && registry.dependency_count == 2);
    for (StaticRegistryUse *item = registry.first_use; item; item = item->next) {
        assert(static_registry_complete_local(&registry, item));
    }
    StaticRegistryCheckState closure;
    assert(static_registry_refresh_checks(&registry, &closure) && closure == STATIC_REGISTRY_CHECKS_CLOSED);
    const StaticCreationPath *unqueried = producer_root(target_module, text);
    assert(registry.use_count == 3);
    assert(static_registry_refresh_checks(&registry, &closure) && closure == STATIC_REGISTRY_CHECKS_PENDING);
    assert(registry.use_count == 4 && registry.candidate_count == 4);
    assert(static_registry_use(&registry, unqueried, &use, &state));
    assert(static_registry_complete_local(&registry, use));
    assert(static_registry_refresh_checks(&registry, &closure) && closure == STATIC_REGISTRY_CHECKS_CLOSED);
    destroy_registry();
}

/** Every allocation in automatic ancestry/dependency retention can be refused
 * without publishing a receiver or leaving a valid certificate in a failed plan.
 */
static void it_releases_partial_ancestry_and_dependencies_at_every_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        (void)setup_producers();
        const StaticCreationPath *a = producer_root(caller_module, integer_type);
        const StaticCreationPath *b = producer_child(a, ".other result", target_module, integer_type);
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        StaticRegistryUse *output = NULL;
        StaticRegistryKeyState state = STATIC_REGISTRY_KEY_PENDING;
        bool result = static_registry_use(&registry, b, &output, &state);
        if (!failure) {
            assert(result && output && registry.dependency_count == 1);
            total = allocation_count - before;
            assert(total);
        } else {
            assert(!result && output == NULL && state == STATIC_REGISTRY_KEY_PENDING);
            assert(registry.status == STATIC_REGISTRY_OUT_OF_MEMORY && diagnostic_valid(&registry.error));
            for (const StaticRegistryCandidate *item = registry.first_candidate; item; item = item->next) {
                assert(item->state == STATIC_CANDIDATE_FAILED);
            }
        }
        failure_at = 0;
        destroy_registry();
    }
}

/** Execute cached-grandchild and same-key-cycle-branch compositions. */
static void run(void) {
    it_releases_partial_ancestry_and_dependencies_at_every_allocation_refusal();
    it_retains_guard_demanded_paths_and_ancestors_without_caller_enumeration();
    it_rechecks_warm_cached_grandchildren_in_both_root_orders();
    it_checks_new_outgoing_branches_of_a_checked_same_key_cycle();
}
/** Return after counted teardown of all source/name/guard/registry owners. */
int main(void) { run(); return 0; }
