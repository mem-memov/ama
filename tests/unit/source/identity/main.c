/**
 * @file
 * @brief Check exact qualified identity encoding, SHA padding vectors and collision rejection.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include "../common.c"
#include "source/identity/identity.h"
#include "source/identity/internal.h"
#include "source/identity/hash/sha256.h"
#include "vectors.inc"
#include "support/roots.c"
#include "support/failures.c"
#include "support/diagnostics.c"

/**
 * Encode all 32 digest bytes as 64 lowercase hexadecimal digits in caller-owned writable storage,
 * followed by a terminator for independent expected-vector comparison.
 */
static void digest_text(const unsigned char digest[32], char text[65]) {
    const char *digits = "0123456789abcdef";
    for (size_t index = 0; index < 32; index++) {
        text[index * 2] = digits[digest[index] >> 4];
        text[index * 2 + 1] = digits[digest[index] & 15];
    }
    text[64] = 0;
}

/**
 * Assert that the component matches sha256 vectors across padding and block boundaries. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_matches_sha256_vectors_across_padding_and_block_boundaries(void) {
    unsigned char bytes[129];
    for (size_t index = 0; index < sizeof(bytes); index++) {
        bytes[index] = (unsigned char)index;
    }
    for (size_t index = 0; index < sizeof(padding_vectors) / sizeof(padding_vectors[0]); index++) {
        unsigned char digest[32];
        char text[65];
        identity_sha256(bytes, padding_vectors[index].length, digest);
        digest_text(digest, text);
        assert(strcmp(text, padding_vectors[index].digest) == 0);
    }
    unsigned char digest[32];
    char text[65];
    identity_sha256((const unsigned char *)"abc", 3, digest);
    digest_text(digest, text);
    assert(strcmp(text, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad") == 0);
}

/**
 * Assert that the component preserves unicode names and matches a stable full identity vector.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_preserves_unicode_names_and_matches_a_stable_full_identity_vector(void) {
    setup_test();
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    const char *members[] = {"价格"};
    QualifiedIdentity identity = {IDENTITY_STATE_MEMBER, ".商店.商品", members, 1, {0}};
    const IdentityName *name = NULL;
    IdentityStatus status = identity_register(&registry, identity, &name);
    assert(status == IDENTITY_OK);
    assert(strcmp(name->source_name, ".商店.商品::价格") == 0);
    assert(strcmp(name->internal_name, "ama_s_a56eb903510b813e736a527a0a51192611a9a1e8adbd577c4e58dc31ddead8a3") == 0);
    const IdentityName *again = NULL;
    status = identity_register(&registry, identity, &again);
    assert(status == IDENTITY_OK && again == name);
    identity_registry_destroy(&registry);
    destroy_test();
}

/**
 * Assert that the component separates categories components case and unicode spelling. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_separates_categories_components_case_and_unicode_spelling(void) {
    setup_test();
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    const char *members[] = {"c"};
    QualifiedIdentity identities[] = {
        {IDENTITY_TYPE, ".ab.c", NULL, 0, {0}},
        {IDENTITY_TYPE, ".a.bc", NULL, 0, {0}},
        {IDENTITY_TYPE, ".ab", members, 1, {0}},
        {IDENTITY_TYPE, ".Ab.c", NULL, 0, {0}},
        {IDENTITY_TYPE, ".é", NULL, 0, {0}},
        {IDENTITY_TYPE, ".e\xcc\x81", NULL, 0, {0}}
    };
    const IdentityName *names[6];
    for (size_t index = 0; index < 6; index++) {
        IdentityStatus status = identity_register(&registry, identities[index], &names[index]);
        assert(status == IDENTITY_OK);
        for (size_t previous = 0; previous < index; previous++) {
            assert(strcmp(names[previous]->internal_name, names[index]->internal_name) != 0);
        }
    }
    for (int category = IDENTITY_CONSTANT; category <= IDENTITY_NATIVE_NAMESPACE; category++) {
        QualifiedIdentity identity = identities[0];
        identity.category = (IdentityCategory)category;
        const IdentityName *name;
        IdentityStatus status = identity_register(&registry, identity, &name);
        assert(status == IDENTITY_OK && strcmp(name->internal_name, names[0]->internal_name) != 0);
    }
    identity_registry_destroy(&registry);
    destroy_test();
}

/**
 * Assert that the component refuses a forced digest collision without merging full keys. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_refuses_a_forced_digest_collision_without_merging_full_keys(void) {
    setup_test();
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    QualifiedIdentity first = {IDENTITY_TYPE, ".one", NULL, 0, {0}};
    QualifiedIdentity second = {IDENTITY_TYPE, ".two", NULL, 0, {0}};
    IdentityEncoding encoded_first;
    IdentityEncoding encoded_second;
    IdentityStatus status = identity_encode(&registry.arena, first, &encoded_first);
    assert(status == IDENTITY_OK);
    status = identity_encode(&registry.arena, second, &encoded_second);
    assert(status == IDENTITY_OK);
    unsigned char digest[32] = {0};
    const IdentityName *name = NULL;
    status = identity_insert(&registry, encoded_first, digest, &name);
    assert(status == IDENTITY_OK);
    const IdentityName *output = NULL;
    status = identity_insert(&registry, encoded_second, digest, &output);
    assert(status == IDENTITY_COLLISION && output == NULL);
    assert(registry.conflicting == name && registry.first == name && name->next == NULL);
    identity_registry_destroy(&registry);
    destroy_test();
}

/**
 * Assert that the component releases every failed registration and rejects invalid identities.
 * The explicit fixture setup and observations define this regression; assertion failure keeps
 * its generated evidence.
 */
static void it_releases_every_failed_registration_and_rejects_invalid_identities(void) {
    setup_test();
    IdentityRegistry measured;
    identity_registry_init(&measured, sources.allocator);
    const char *parts[] = {"price"};
    const IdentityName *complete = NULL;
    assert(identity_register(&measured, (QualifiedIdentity){IDENTITY_STATE_MEMBER, ".shop.item", parts, 1, {0}}, &complete) == IDENTITY_OK);
    size_t count = allocation_count;
    identity_registry_destroy(&measured);
    destroy_test();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_test();
        IdentityRegistry registry;
        identity_registry_init(&registry, sources.allocator);
        failure_at = failure;
        const IdentityName *name = NULL;
        const char *members[] = {"price"};
        QualifiedIdentity identity = {IDENTITY_STATE_MEMBER, ".shop.item", members, 1, {0}};
        IdentityStatus status = identity_register(&registry, identity, &name);
        assert(status == IDENTITY_OUT_OF_MEMORY && name == NULL && registry.first == NULL);
        failure_at = 0;
        identity.module_path = ".bad..path";
        status = identity_register(&registry, identity, &name);
        assert(status == IDENTITY_INVALID);
        identity_registry_destroy(&registry);
        destroy_test();
    }
}

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_composes_a_real_unequal_key_collision_into_both_original_declarations();
    it_builds_many_member_displays_with_constant_allocation_count();
    it_matches_frozen_root_aware_vectors_through_the_current_identity_producer();
    it_separates_all_categories_roots_and_complete_component_boundaries();
    it_measures_exact_root_shape_and_bounds_every_length_prefix_before_allocation();
    it_rejects_invalid_raw_key_shape_without_publishing_or_dereferencing_missing_data();
    it_preserves_existing_names_through_every_root_aware_registration_allocation_failure();
    it_matches_sha256_vectors_across_padding_and_block_boundaries();
    it_preserves_unicode_names_and_matches_a_stable_full_identity_vector();
    it_separates_categories_components_case_and_unicode_spelling();
    it_refuses_a_forced_digest_collision_without_merging_full_keys();
    it_releases_every_failed_registration_and_rejects_invalid_identities();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
