/**
 * @file
 * @brief Verify category/root/component injectivity, fixed version-two native vectors and discovery-order independence.
 */
#include "source/identity/encoding/internal.h"
#include "../../../native/namespace/support/vectors.c"

/** Current complete source identities must match the independently frozen bytes and SHA prefixes in either order. */
static void it_matches_frozen_root_aware_vectors_through_the_current_identity_producer(void) {
    for (size_t reverse = 0; reverse < 2; reverse++) {
        setup_test();
        IdentityRegistry registry;
        identity_registry_init(&registry, sources.allocator);
        const IdentityName *names[3];
        for (size_t index = 0; index < 3; index++) {
            size_t selected = reverse == 0 ? index : 2 - index;
            names[selected] = register_namespace_vector(&registry, selected);
        }
        for (size_t index = 0; index < 3; index++) {
            assert(register_namespace_vector(&registry, index) == names[index]);
            assert(names[index]->encoding[7] == 2 && names[index]->encoding[8] == IDENTITY_NATIVE_NAMESPACE);
        }
        assert(names[0]->encoding[9] == MODULE_ROOT_PROJECT);
        assert(names[2]->encoding[9] == MODULE_ROOT_STANDARD);
        /* Ordinary identities now also require their complete package prefix;
         * standard identities have no vendor/version wrapper. Frozen vectors
         * above independently verify every count/byte in both complete keys. */
        assert(names[0]->encoding_length > names[2]->encoding_length);
        assert(strcmp(names[0]->internal_name, names[2]->internal_name) != 0);
        identity_registry_destroy(&registry);
        destroy_test();
    }
}

/** All categories and both roots remain distinct even when components, display spelling and C-safe shape overlap. */
static void it_separates_all_categories_roots_and_complete_component_boundaries(void) {
    setup_test();
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    const char *paths[] = {"/test/fixture/v1.0.0/src/item state", "//item state", "/test/fixture/v1.0.0/src/item/state", "/test/fixture/v1.0.0/src/Item state", "/test/fixture/v1.0.0/src/é value", "/test/fixture/v1.0.0/src/e\xcc\x81 value",
        "/test/fixture/v1.0.0/src/a-b", "/test/fixture/v1.0.0/src/a_b", "/test/fixture/v1.0.0/src/a b", "/test/fixture/v1.0.0/src/ab"};
    const IdentityName *names[sizeof(paths) / sizeof(*paths)][7];
    for (size_t path = 0; path < sizeof(paths) / sizeof(*paths); path++) {
        for (int category = IDENTITY_TYPE; category <= IDENTITY_NATIVE_NAMESPACE; category++) {
            QualifiedIdentity identity = {(IdentityCategory)category, paths[path], NULL, 0, {0}};
            assert(identity_register(&registry, identity, &names[path][category - 1]) == IDENTITY_OK);
            assert(strcmp(names[path][category - 1]->source_name, paths[path]) == 0);
            for (size_t previous_path = 0; previous_path <= path; previous_path++) {
                int bound = previous_path == path ? category - 1 : IDENTITY_NATIVE_NAMESPACE;
                for (int previous_category = IDENTITY_TYPE; previous_category <= bound; previous_category++) {
                    assert(strcmp(names[path][category - 1]->internal_name, names[previous_path][previous_category - 1]->internal_name) != 0);
                }
            }
        }
    }
    const char *one[] = {"a::b"}, *two[] = {"a","b"};
    QualifiedIdentity identity = {IDENTITY_STATE_MEMBER, "/test/fixture/v1.0.0/src/item state", one, 1, {0}};
    const IdentityName *first = NULL, *second = NULL;
    assert(identity_register(&registry, identity, &first) == IDENTITY_OK);
    identity.members = two;
    identity.member_count = 2;
    assert(identity_register(&registry, identity, &second) == IDENTITY_OK);
    assert(strcmp(first->source_name, second->source_name) == 0);
    assert(strcmp(first->internal_name, second->internal_name) != 0);
    identity_registry_destroy(&registry);
    destroy_test();
}

/** Full source shape includes a root tag, not a spurious empty Standard component; numeric limits publish no partial size. */
static void it_measures_exact_root_shape_and_bounds_every_length_prefix_before_allocation(void) {
    IdentityEncodingShape shape = {0};
    QualifiedIdentity identity = {IDENTITY_NATIVE_NAMESPACE, "//магазин/товар данные", NULL, 0, {0}};
    assert(identity_encoding_measure(identity, &shape) == IDENTITY_OK);
    assert(shape.length == sizeof(namespace_key_2) && shape.module_count == 2 && shape.root_width == 2 && shape.root_kind == MODULE_ROOT_STANDARD);
    IdentityEncodingShape unchanged = shape;
    identity.module_path = "///bad";
    assert(identity_encoding_measure(identity, &shape) == IDENTITY_INVALID);
    assert(shape.length == unchanged.length && shape.module_count == unchanged.module_count);
    assert(shape.display_length == unchanged.display_length);
    assert(shape.root_width == unchanged.root_width && shape.root_kind == unchanged.root_kind);
    size_t limit = SIZE_MAX < UINT64_MAX / 8 ? SIZE_MAX : (size_t)(UINT64_MAX / 8);
    size_t total = limit - 8;
    assert(identity_encoding_part_size(&total, 0) && total == limit);
    assert(!identity_encoding_part_size(&total, 0) && total == limit);
    total = limit - 9;
    assert(identity_encoding_part_size(&total, 1) && total == limit);
    total = limit - 8;
    assert(!identity_encoding_part_size(&total, 1) && total == limit - 8);
    total = SIZE_MAX - 7;
    assert(!identity_encoding_part_size(&total, 0) && total == SIZE_MAX - 7);
    total = 0;
    assert(!identity_encoding_part_size(&total, SIZE_MAX) && total == 0);
    assert(!identity_encoding_part_size(NULL, 0));
}
