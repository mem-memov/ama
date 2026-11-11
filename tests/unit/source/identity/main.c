#include "../common.c"
#include "source/identity/identity.h"
#include "source/identity/internal.h"
#include "source/identity/hash/sha256.h"
#include "vectors.inc"

static void digest_text(const unsigned char digest[32], char text[65]) {
    const char *digits = "0123456789abcdef";
    for (size_t index = 0; index < 32; index++) {
        text[index * 2] = digits[digest[index] >> 4];
        text[index * 2 + 1] = digits[digest[index] & 15];
    }
    text[64] = 0;
}

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
    assert(strcmp(name->internal_name, "ama_s_9850ad5a414497be44fc24f05843deca415278702c4045e0deacd8ea0e2c1802") == 0);
    const IdentityName *again = NULL;
    status = identity_register(&registry, identity, &again);
    assert(status == IDENTITY_OK && again == name);
    identity_registry_destroy(&registry);
    destroy_test();
}

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

static void it_releases_every_failed_registration_and_rejects_invalid_identities(void) {
    for (size_t failure = 1; failure <= 4; failure++) {
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

void run(void) {
    it_matches_sha256_vectors_across_padding_and_block_boundaries();
    it_preserves_unicode_names_and_matches_a_stable_full_identity_vector();
    it_separates_categories_components_case_and_unicode_spelling();
    it_refuses_a_forced_digest_collision_without_merging_full_keys();
    it_releases_every_failed_registration_and_rejects_invalid_identities();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
