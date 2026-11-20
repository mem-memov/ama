/** @file
 * @brief Compose actual source/term/guard paths with candidate interning and pending key promotion.
 */
#include "../common.c"
#include "source/identity/internal.h"
#include "source/identity/hash/sha256.h"

/** Equal closed maps share one identity/name but preserve two actual source paths.
 * Naming leaves the candidate NEW; it is not a body/executable certificate.
 */
static void it_shares_equal_candidate_keys_without_merging_incoming_uses(void) {
    setup_registry("@<\nvalue\n.type\n@!\nanchor&\n//integer\n", "@!\n");
    StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
    const StaticTermEnvironment *concrete = key_ordinary(caller_module, &input, 1);
    const StaticCreationPath *first_path, *second_path;
    assert(static_creation_root(&guards, concrete, &first_path));
    const StaticTerm *variable;
    assert(static_term_symbol(&terms, caller_environment, input.formal, &variable));
    assert(static_constraint_equate(&solver, variable, integer_type, input.formal->site));
    assert(static_creation_root(&guards, caller_environment, &second_path));
    assert(first_path != second_path && first_path->vertex == second_path->vertex);
    StaticRegistryUse *first, *second, *again;
    StaticRegistryKeyState state;
    assert(static_registry_use(&registry, first_path, &first, &state) && state == STATIC_REGISTRY_KEY_CLOSED);
    assert(static_registry_use(&registry, second_path, &second, &state) && state == STATIC_REGISTRY_KEY_CLOSED);
    assert(first != second && first->candidate == second->candidate);
    assert(first->path == first_path && second->path == second_path);
    assert(registry.use_count == 2 && registry.candidate_count == 1);
    assert(first->candidate->state == STATIC_CANDIDATE_NEW && registry_names.first->next == NULL);
    size_t before = allocation_count;
    assert(static_registry_use(&registry, first_path, &again, &state) && again == first);
    assert(allocation_count == before && registry.use_count == 2 && registry.candidate_count == 1);
    destroy_registry();
}

/** An unknown owner keeps a stable use handle without a fabricated name/key;
 * actual equations later promote that same path to a complete candidate.
 */
static void it_promotes_pending_uses_only_after_independent_complete_key_evidence(void) {
    setup_registry("@<\nvalue\n.type\n@!\nanchor&\n//integer\n", "@!\n");
    const StaticCreationPath *path;
    assert(static_creation_root(&guards, caller_environment, &path));
    StaticRegistryUse *use, *again;
    StaticRegistryKeyState state;
    assert(static_registry_use(&registry, path, &use, &state) && state == STATIC_REGISTRY_KEY_PENDING);
    assert(use->candidate == NULL && registry_names.first == NULL && registry.candidate_count == 0);
    assert(static_registry_use(&registry, path, &again, &state) && again == use);
    const StaticTerm *variable;
    const StaticSymbol *formal = application_formal(caller_module, 0);
    assert(static_term_symbol(&terms, caller_environment, formal, &variable));
    assert(static_constraint_equate(&solver, variable, integer_type, formal->site));
    assert(static_registry_refresh_keys(&registry, &state) && state == STATIC_REGISTRY_KEY_CLOSED);
    assert(use->path == path && use->candidate && use->key_state == STATIC_REGISTRY_KEY_CLOSED);
    assert(registry.use_count == 1 && registry.candidate_count == 1);
    assert(use->candidate->state == STATIC_CANDIDATE_NEW);
    destroy_registry();
}

/** Every measured allocation can fail without publishing a public use result;
 * internally linked partial uses remain unconditionally destructible.
 */
static void it_unwinds_every_use_and_candidate_preparation_allocation_refusal(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_registry("@<\nvalue\n.type\n@!\nanchor&\n//integer\n", "@!\n");
        StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
        const StaticCreationPath *path;
        assert(static_creation_root(&guards, key_ordinary(caller_module, &input, 1), &path));
        size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        StaticRegistryUse *output = NULL;
        StaticRegistryKeyState state = STATIC_REGISTRY_KEY_PENDING;
        bool success = static_registry_use(&registry, path, &output, &state);
        if (!failure) { assert(success && output); total = allocation_count - before; assert(total); }
        else {
            assert(!success && output == NULL && state == STATIC_REGISTRY_KEY_PENDING);
            assert(registry.status == STATIC_REGISTRY_OUT_OF_MEMORY && diagnostic_valid(&registry.error));
        }
        failure_at = 0;
        destroy_registry();
    }
}

/** Foreign path copies and missing receivers fail before candidate publication. */
static void it_refuses_foreign_paths_and_missing_receivers_with_sticky_diagnostics(void) {
    for (size_t partition = 0; partition < 4; partition++) {
        setup_registry("@!\nanchor&\n//integer\n", "@!\n");
        const StaticCreationPath *path;
        assert(static_creation_root(&guards, caller_environment, &path));
        StaticCreationPath copied = *path;
        StaticRegistryUse *output = NULL;
        StaticRegistryKeyState state = STATIC_REGISTRY_KEY_PENDING;
        size_t before = allocation_count;
        if (partition == 3) { assert(!static_registry_refresh_keys(&registry, NULL)); }
        else {
            assert(!static_registry_use(&registry, partition == 0 ? &copied : path,
                partition == 1 ? NULL : &output, partition == 2 ? NULL : &state));
        }
        assert(output == NULL && registry.status == STATIC_REGISTRY_INVALID && allocation_count == before);
        const char *message = registry.error.message;
        assert(!static_registry_refresh_keys(&registry, &state) && registry.error.message == message);
        destroy_registry();
    }
}

/** Refuse a forged digest match through the actual candidate handoff rather
 * than treating its existing name record as the requested specialization.
 */
static void it_propagates_full_key_name_collisions_without_publishing_candidates(void) {
    setup_registry("@<\nvalue\n.type\n@!\nanchor&\n//integer\n", "@!\n");
    StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
    const StaticTermEnvironment *key = key_ordinary(caller_module, &input, 1);
    const StaticCreationPath *path;
    assert(static_creation_root(&guards, key, &path));
    SourceArena scratch;
    source_arena_init(&scratch, sources.allocator);
    StaticKeyEncoding encoded = key_encode(&scratch, key);
    IdentityEncoding actual, other;
    assert(identity_encode(&registry_names.arena, encoded.qualified, &actual) == IDENTITY_OK);
    unsigned char digest[32];
    identity_sha256(actual.bytes, actual.length, digest);
    QualifiedIdentity fake = {.category = IDENTITY_TYPE, .module_path = "/test/fixture/v1.0.0/src/other"};
    assert(identity_encode(&registry_names.arena, fake, &other) == IDENTITY_OK);
    const IdentityName *existing;
    assert(identity_insert(&registry_names, other, digest, &existing) == IDENTITY_OK);
    StaticRegistryUse *output = NULL;
    StaticRegistryKeyState state = STATIC_REGISTRY_KEY_PENDING;
    assert(!static_registry_use(&registry, path, &output, &state));
    assert(output == NULL && state == STATIC_REGISTRY_KEY_PENDING);
    assert(registry.status == STATIC_REGISTRY_COLLISION && registry.candidate_count == 0);
    assert(registry_names.conflicting == existing && registry.error.code == DIAGNOSTIC_IDENTITY_COLLISION);
    source_arena_destroy(&scratch);
    destroy_registry();
}

/** Dense use/candidate ordinals refuse representational overflow rather than
 * wrapping into an existing identity; all partial records remain destructible.
 */
static void it_refuses_use_and_candidate_counter_overflow_before_publication(void) {
    for (unsigned partition = 0; partition < 2; partition++) {
        setup_registry("@<\nvalue\n.type\n@!\nanchor&\n//integer\n", "@!\n");
        StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
        const StaticCreationPath *path;
        assert(static_creation_root(&guards, key_ordinary(caller_module, &input, 1), &path));
        if (partition == 0) { registry.use_count = SIZE_MAX; }
        else { registry.candidate_count = SIZE_MAX; }
        StaticRegistryUse *output = NULL;
        StaticRegistryKeyState state = STATIC_REGISTRY_KEY_PENDING;
        assert(!static_registry_use(&registry, path, &output, &state));
        assert(output == NULL && state == STATIC_REGISTRY_KEY_PENDING);
        assert(registry.status == STATIC_REGISTRY_OUT_OF_MEMORY && registry_names.first == NULL);
        assert(diagnostic_valid(&registry.error));
        destroy_registry();
    }
}

/** Execute identity sharing, pending promotion, all refusal sites and membership. */
static void run(void) {
    it_refuses_use_and_candidate_counter_overflow_before_publication();
    it_propagates_full_key_name_collisions_without_publishing_candidates();
    it_shares_equal_candidate_keys_without_merging_incoming_uses();
    it_promotes_pending_uses_only_after_independent_complete_key_evidence();
    it_unwinds_every_use_and_candidate_preparation_allocation_refusal();
    it_refuses_foreign_paths_and_missing_receivers_with_sticky_diagnostics();
}
/** Return success after counted teardown. */
int main(void) { run(); return 0; }
