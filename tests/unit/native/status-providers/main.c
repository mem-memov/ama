/**
 * @file
 * @brief Exercise exact full-key provider ordering, transaction failure and namespace capacity.
 */
#include "../../source/common.c"
#include "native/status/providers/internal.h"
#include "../namespace/support/vectors.c"

/** Observe stable assignments across every permutation; caller inventory remains untouched. */
static void it_assigns_namespaces_by_complete_keys_independent_of_discovery_order(void) {
    setup_test();
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    const IdentityName *names[3];
    for (size_t index = 0; index < 3; index++) { names[index] = register_namespace_vector(&registry, index); }
    const size_t permutations[][3] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
    for (size_t selected = 0; selected < 6; selected++) {
        const IdentityName *inventory[3];
        for (size_t index = 0; index < 3; index++) { inventory[index] = names[permutations[selected][index]]; }
        NativeStatusProviders owner;
        native_status_providers_init(&owner, sources.allocator);
        assert(native_status_providers_build(&owner, inventory, 3).code == AMA_OK);
        assert(owner.ready && owner.count == 3);
        for (size_t index = 0; index < 3; index++) {
            uint16_t found = 0;
            assert(owner.providers[index].name == names[index]);
            assert(native_status_provider_find(&owner, names[index], &found).code == AMA_OK && found == index + 1);
            assert(inventory[index] == names[permutations[selected][index]]);
        }
        IdentityName equivalent = *names[1];
        memset(equivalent.digest, 0, sizeof(equivalent.digest));
        uint16_t found = 88;
        assert(native_status_provider_find(&owner, &equivalent, &found).code == AMA_OK && found == 2);
        equivalent.encoding_length--;
        assert(native_status_provider_find(&owner, &equivalent, &found).code == AMA_TYPE_MISMATCH && found == 2);
        equivalent.category = IDENTITY_TYPE;
        assert(native_status_provider_find(&owner, &equivalent, &found).code == AMA_INVALID_STATE && found == 2);
        assert(native_status_providers_build(&owner, inventory, 3).code == AMA_INVALID_STATE);
        native_status_providers_destroy(&owner);
        native_status_providers_destroy(&owner);
    }
    identity_registry_destroy(&registry);
    destroy_test();
}

/** Force an actual digest collision: exact-key insertion diagnoses both identities, never merges. */
static void it_rejects_hash_collisions_before_namespace_planning(void) {
    setup_test();
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    const IdentityName *first = register_namespace_vector(&registry, 0);
    unsigned char *bytes = source_arena_allocate(&registry.arena, sizeof(namespace_key_2));
    assert(bytes != NULL);
    memcpy(bytes, namespace_key_2, sizeof(namespace_key_2));
    IdentityEncoding encoding = {bytes, sizeof(namespace_key_2), "different root", IDENTITY_NATIVE_NAMESPACE, {0}};
    const IdentityName *unchanged = first;
    assert(identity_insert(&registry, encoding, first->digest, &unchanged) == IDENTITY_COLLISION);
    assert(unchanged == first && registry.conflicting == first && strcmp(encoding.source_name, first->source_name) != 0);
    assert(registry.first == first && first->next == NULL);
    identity_registry_destroy(&registry);
    destroy_test();
}

/** Reject duplicate and malformed inventory without exposing partially assigned rows. */
static void it_keeps_invalid_provider_plans_unpublished(void) {
    setup_test();
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    const IdentityName *name = register_namespace_vector(&registry, 0);
    IdentityName malformed = *name;
    const IdentityName *inventories[][2] = {{name,name},{name,NULL},{name,&malformed}};
    for (size_t selected = 0; selected < 5; selected++) {
        if (selected == 2) { malformed.category = IDENTITY_TYPE; }
        if (selected == 3) { malformed.category = IDENTITY_NATIVE_NAMESPACE; malformed.encoding = NULL; }
        if (selected == 4) { malformed.encoding = name->encoding; malformed.encoding_length = 0; }
        NativeStatusProviders owner;
        native_status_providers_init(&owner, sources.allocator);
        assert(native_status_providers_build(&owner, inventories[selected < 2 ? selected : 2], 2).code == AMA_INVALID_STATE);
        assert(!owner.ready && owner.count == 0 && owner.providers == NULL);
        if (selected == 0) { assert(owner.conflict_first == name && owner.conflict_second == name); }
        uint16_t output = 91;
        assert(native_status_provider_find(&owner, name, &output).code == AMA_INVALID_STATE && output == 91);
        native_status_providers_destroy(&owner);
    }
    NativeStatusProviders owner;
    native_status_providers_init(&owner, sources.allocator);
    assert(native_status_providers_build(&owner, NULL, 1).code == AMA_INVALID_STATE);
    native_status_providers_destroy(&owner);
    identity_registry_destroy(&registry);
    destroy_test();
}

/** Check exact namespace endpoints and every real planning allocation failure, then fresh recovery. */
static void it_bounds_provider_capacity_and_rolls_back_allocation_failure(void) {
    setup_test();
    NativeStatusProviders owner;
    native_status_providers_init(&owner, sources.allocator);
    size_t before = allocation_count;
    assert(native_status_providers_build(&owner, NULL, 0).code == AMA_OK && owner.ready && owner.count == 0);
    assert(allocation_count == before);
    native_status_providers_destroy(&owner);
    const size_t excessive[] = {(size_t)UINT16_MAX + 1, SIZE_MAX};
    for (size_t index = 0; index < 2; index++) {
        native_status_providers_init(&owner, sources.allocator);
        assert(native_status_providers_build(&owner, NULL, excessive[index]).code == AMA_LIMIT_EXCEEDED);
        assert(!owner.ready && owner.providers == NULL && allocation_count == before);
        native_status_providers_destroy(&owner);
    }
    uint16_t ordinal = 0;
    assert(native_status_provider_ordinal(0, &ordinal).code == AMA_OK && ordinal == 1);
    assert(native_status_provider_ordinal(UINT16_MAX - 1, &ordinal).code == AMA_OK && ordinal == UINT16_MAX);
    assert(native_status_provider_ordinal(UINT16_MAX, &ordinal).code == AMA_LIMIT_EXCEEDED && ordinal == UINT16_MAX);
    assert(native_status_provider_ordinal(SIZE_MAX, &ordinal).code == AMA_LIMIT_EXCEEDED && ordinal == UINT16_MAX);
    assert(native_status_provider_ordinal(0, NULL).code == AMA_INVALID_STATE);
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    const IdentityName *names[] = {register_namespace_vector(&registry, 2),register_namespace_vector(&registry, 0)};
    native_status_providers_init(&owner, sources.allocator);
    before = allocation_count;
    assert(native_status_providers_build(&owner, names, 2).code == AMA_OK);
    size_t calls = allocation_count - before;
    assert(calls != 0);
    native_status_providers_destroy(&owner);
    size_t baseline = live_allocations;
    for (size_t selected = 1; selected <= calls; selected++) {
        native_status_providers_init(&owner, sources.allocator);
        failure_at = allocation_count + selected;
        assert(native_status_providers_build(&owner, names, 2).code == AMA_OUT_OF_MEMORY);
        assert(!owner.ready && owner.providers == NULL && owner.count == 0);
        failure_at = 0;
        native_status_providers_destroy(&owner);
        assert(live_allocations == baseline);
    }
    native_status_providers_init(&owner, sources.allocator);
    assert(native_status_providers_build(&owner, names, 2).code == AMA_OK);
    native_status_providers_destroy(&owner);
    identity_registry_destroy(&registry);
    destroy_test();
}

/** Exhaust every ordering up to eight distinct keys, exercising odd/even heap frontiers. */
static void it_sorts_every_small_inventory_permutation_exactly(void) {
    IdentityName names[8] = {{0}};
    unsigned char keys[8];
    for (size_t index = 0; index < 8; index++) {
        keys[index] = (unsigned char)index;
        names[index].encoding = &keys[index];
        names[index].encoding_length = 1;
    }
    for (size_t count = 0; count <= 8; count++) {
        size_t permutation[8];
        for (size_t index = 0; index < count; index++) { permutation[index] = index; }
        for (;;) {
            NativeStatusProvider rows[8];
            for (size_t index = 0; index < count; index++) { rows[index].name = &names[permutation[index]]; }
            native_status_providers_order(rows, count);
            for (size_t index = 0; index < count; index++) { assert(rows[index].name == &names[index]); }
            size_t pivot = count;
            while (pivot > 1 && permutation[pivot - 2] >= permutation[pivot - 1]) { pivot--; }
            if (pivot <= 1) { break; }
            size_t selected = count;
            while (permutation[selected - 1] <= permutation[pivot - 2]) { selected--; }
            size_t swapped = permutation[pivot - 2];
            permutation[pivot - 2] = permutation[selected - 1];
            permutation[selected - 1] = swapped;
            for (size_t left = pivot - 1, right = count - 1; left < right; left++, right--) {
                swapped = permutation[left]; permutation[left] = permutation[right]; permutation[right] = swapped;
            }
        }
    }
}

/** Run all component and registry-composition cases explicitly. */
void run(void) {
    it_assigns_namespaces_by_complete_keys_independent_of_discovery_order();
    it_rejects_hash_collisions_before_namespace_planning();
    it_keeps_invalid_provider_plans_unpublished();
    it_bounds_provider_capacity_and_rolls_back_allocation_failure();
    it_sorts_every_small_inventory_permutation_exactly();
}

/** Return success only after every invariant observation passes. */
int main(void) { run(); return EXIT_SUCCESS; }
