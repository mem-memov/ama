/** @file
 * @brief Check key validation, arithmetic, exact hash collisions and every observed allocation refusal.
 */
#include "../common.c"
#include "source/identity/internal.h"
#include "source/identity/hash/sha256.h"
#include <stdio.h>

/** Rejected domains change no public receiver or borrowed term-store state. */
static void it_refuses_open_foreign_and_malformed_owner_keys_without_publication(void) {
    for (size_t partition = 0; partition < 8; partition++) {
        setup_guards("@<\nvalue\n.type\n@!\nanchor&\n//integer\n",
            "@<\nvalue\n.other\n@!\n");
        StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
        const StaticTermEnvironment *environment = key_ordinary(caller_module, &input, 1);
        StaticTermEnvironment copied = *environment;
        if (partition == 3) { environment = caller_environment; }
        if (partition == 4 || partition == 5) {
            if (partition == 4) {
                assert(static_term_symbol(&terms, caller_environment, input.formal, &input.value));
            } else { input.value = target_constructor; }
            const StaticTerm *raw;
            assert(static_term_nominal(&terms, caller_module, &input, 1, (SourceSpan){0}, &raw));
            assert(static_term_environment(&terms, caller_module, raw, &environment));
        }
        SourceArena destination;
        source_arena_init(&destination, sources.allocator);
        StaticKeyEncoding output = {.descriptor = "unchanged", .length = 42,
            .qualified = {.module_path = "unchanged"}};
        StaticKeyEncoding before_output = output;
        size_t before = allocation_count, count = terms.term_count, environments = terms.environment_count;
        StaticKeyStatus status = static_key_encode(partition == 0 ? NULL : &terms,
            partition == 2 ? &copied : environment,
            partition == 6 ? &terms.arena : partition == 7 ? NULL : &destination,
            partition == 1 ? NULL : &output);
        assert(status == STATIC_KEY_INVALID && allocation_count == before);
        assert(output.descriptor == before_output.descriptor && output.length == before_output.length);
        assert(output.qualified.module_path == before_output.qualified.module_path);
        assert(terms.term_count == count && terms.environment_count == environments && terms.status == STATIC_TERM_OK);
        source_arena_destroy(&destination);
        destroy_guards();
    }
    setup_guards("@...\n.anchor\n//integer\n", "@!\n");
    SourceArena destination;
    source_arena_init(&destination, sources.allocator);
    StaticKeyEncoding output = {.descriptor = "unchanged", .length = 42};
    size_t before = allocation_count;
    assert(static_key_encode(&terms, caller_environment, &destination, &output) == STATIC_KEY_INVALID);
    assert(allocation_count == before && output.length == 42 && strcmp(output.descriptor, "unchanged") == 0);
    source_arena_destroy(&destination);
    destroy_guards();
}

/** Every actual discovery/destination allocation fails once. Partial destination
 * storage stays owned, and a later retry is safe without replacing source/terms.
 */
static void it_unwinds_every_key_encoding_allocation_and_preserves_safe_retry(void) {
    for (size_t kind = 0; kind < 4; kind++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            bool producer = kind % 2 != 0;
            bool empty = kind >= 2;
            const char *caller = producer ? (empty ? "@...\n.anchor\n//integer\n" :
                "@...\n.echo\n.argument\n.anchor\n//integer\n") :
                (empty ? "@!\nanchor&\n//integer\n" : "@<\nvalue\n.type\n@!\nanchor&\n//integer\n");
            setup_guards(caller, "@<\nfirst\n.a\nsecond\n.b\n@!\n");
            const StaticTermEnvironment *environment = caller_environment;
            if (!empty) {
                const StaticTerm *value = producer ? target_constructor : integer_type;
                if (!producer) {
                    for (size_t depth = 0; depth < 4; depth++) {
                        StaticTermCapture map[] = {{.formal = application_formal(target_module, 0), .value = value},
                            {.formal = application_formal(target_module, 1), .value = value}};
                        assert(static_term_nominal(&terms, target_module, map, 2, (SourceSpan){0}, &value));
                    }
                }
                StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = value};
                if (producer) { assert(static_term_producer_environment(&terms, caller_module, &input, 1, &environment)); }
                else { environment = key_ordinary(caller_module, &input, 1); }
            }
            environment = key_environment(environment);
            SourceArena destination;
            source_arena_init(&destination, sources.allocator);
            StaticKeyEncoding output = {.descriptor = "unchanged", .length = 42};
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            StaticKeyStatus status = static_key_encode(&terms, environment, &destination, &output);
            if (!failure) { assert(status == STATIC_KEY_OK); total = allocation_count - before; assert(total); }
            else {
                assert(status == STATIC_KEY_OUT_OF_MEMORY && output.length == 42);
                assert(strcmp(output.descriptor, "unchanged") == 0 && terms.status == STATIC_TERM_OK);
                failure_at = 0;
                assert(static_key_encode(&terms, environment, &destination, &output) == STATIC_KEY_OK);
                assert(strlen(output.descriptor) == output.length);
            }
            failure_at = 0;
            source_arena_destroy(&destination);
            destroy_guards();
        }
    }
}

/** Writer limits preserve untouched storage, and counts match an independent
 * libc decimal conversion, including the complete size_t boundary.
 */
static void it_checks_counted_writer_arithmetic_and_exact_field_boundaries(void) {
    const size_t counts[] = {0, 1, 9, 10, 99, 100, SIZE_MAX};
    for (size_t index = 0; index < sizeof(counts) / sizeof(*counts); index++) {
        char actual[128], expected[128];
        StaticKeyWriter writer = {.data = actual, .capacity = sizeof(actual) - 1};
        static_key_number(&writer, counts[index]);
        assert(writer.status == STATIC_KEY_OK);
        actual[writer.length] = 0;
        assert(snprintf(expected, sizeof(expected), "%zu:", counts[index]) > 0);
        assert(strcmp(actual, expected) == 0);
    }
    char bytes[4] = {'a', 'b', 'c', 'd'};
    StaticKeyWriter limited = {.data = bytes, .length = 2, .capacity = 3};
    static_key_bytes(&limited, "xy", 2);
    static_key_number(&limited, 123);
    assert(limited.status == STATIC_KEY_LIMIT && limited.length == 2 && memcmp(bytes, "abcd", 4) == 0);
    StaticKeyWriter overflow = {.length = SIZE_MAX - 1};
    static_key_bytes(&overflow, "xy", 2);
    assert(overflow.status == STATIC_KEY_LIMIT && overflow.length == SIZE_MAX - 1);
    char left[32], right[32];
    StaticKeyWriter first = {.data = left, .capacity = sizeof(left)}, second = {.data = right, .capacity = sizeof(right)};
    static_key_text(&first, "a", 1);
    static_key_text(&first, " b:c", 4);
    static_key_text(&second, "a ", 2);
    static_key_text(&second, "b:c", 3);
    assert(first.status == STATIC_KEY_OK && second.status == STATIC_KEY_OK);
    assert(first.length != second.length || memcmp(left, right, first.length) != 0);
}

/** The discovery counter refuses wrap before assigning a node ordinal. */
static void it_refuses_postorder_counter_overflow_with_owned_partial_scratch(void) {
    setup_guards("@<\nvalue\n.type\n@!\nanchor&\n//integer\n", "@!\n");
    StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
    StaticKeyPlan plan = {.terms = &terms, .environment = key_ordinary(caller_module, &input, 1), .count = SIZE_MAX};
    source_arena_init(&plan.scratch, sources.allocator);
    size_t live = live_allocations;
    assert(static_key_collect(&plan) == STATIC_KEY_LIMIT && plan.first == NULL && plan.count == SIZE_MAX);
    source_arena_destroy(&plan.scratch);
    assert(live_allocations == live);
    destroy_guards();
}

/** Seed the existing registry with a deliberately forged matching digest but
 * a distinct full source identity. The actual static name must be refused.
 */
static void it_rejects_injected_digest_collisions_without_merging_static_identities(void) {
    setup_guards("@<\nvalue\n.type\n@!\nanchor&\n//integer\n", "@!\n");
    StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
    SourceArena destination;
    source_arena_init(&destination, sources.allocator);
    StaticKeyEncoding key = key_encode(&destination, key_ordinary(caller_module, &input, 1));
    IdentityRegistry names;
    identity_registry_init(&names, sources.allocator);
    IdentityEncoding actual, other;
    assert(identity_encode(&names.arena, key.qualified, &actual) == IDENTITY_OK);
    unsigned char digest[32];
    identity_sha256(actual.bytes, actual.length, digest);
    QualifiedIdentity fake = {.category = IDENTITY_TYPE, .module_path = "/test/fixture/v1.0.0/src/other"};
    assert(identity_encode(&names.arena, fake, &other) == IDENTITY_OK);
    const IdentityName *existing, *output = NULL;
    assert(identity_insert(&names, other, digest, &existing) == IDENTITY_OK);
    assert(static_key_register(&terms, key_ordinary(caller_module, &input, 1), &names, &output) == STATIC_KEY_COLLISION);
    assert(output == NULL && names.conflicting == existing && names.first == existing && existing->next == NULL);
    size_t before = allocation_count;
    assert(static_key_register(NULL, NULL, &names, &output) == STATIC_KEY_INVALID);
    assert(allocation_count == before && names.conflicting == NULL && names.first == existing);
    identity_registry_destroy(&names);
    source_arena_destroy(&destination);
    destroy_guards();
}

/** Complete naming owns only persistent copied identities after temporary
 * codec teardown. Every measured failure keeps the public name unmodified.
 */
static void it_unwinds_full_name_registration_and_retains_only_owned_name_data(void) {
    for (size_t producer = 0; producer < 2; producer++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_guards(producer ? "@...\n.anchor\n//integer\n" :
                "@<\nvalue\n.type\n@!\nanchor&\n//integer\n", "@!\n");
            const StaticTermEnvironment *environment = caller_environment;
            if (!producer) {
                StaticTermCapture input = {.formal = application_formal(caller_module, 0), .value = integer_type};
                environment = key_ordinary(caller_module, &input, 1);
            }
            environment = key_environment(environment);
            IdentityRegistry names;
            identity_registry_init(&names, sources.allocator);
            const IdentityName *output = NULL;
            size_t before = allocation_count;
            if (failure) { failure_at = before + failure; }
            StaticKeyStatus status = static_key_register(&terms, environment, &names, &output);
            if (!failure) { assert(status == STATIC_KEY_OK && output); total = allocation_count - before; assert(total); }
            else {
                assert(status == STATIC_KEY_OUT_OF_MEMORY && output == NULL && names.first == NULL);
                failure_at = 0;
                assert(static_key_register(&terms, environment, &names, &output) == STATIC_KEY_OK);
            }
            assert(output == names.first && output->encoding_length > 0);
            assert(strstr(output->source_name, "AMA-STATIC-KEY:1;") != NULL);
            failure_at = 0;
            identity_registry_destroy(&names);
            destroy_guards();
        }
    }
}

/** Invalid naming arguments are refused before creating a name, and each
 * attempt clears only the prior collision witness while preserving records.
 */
static void it_refuses_invalid_naming_arguments_without_partial_names(void) {
    setup_guards("@!\nanchor&\n//integer\n", "@!\n");
    const StaticTermEnvironment *environment = key_environment(caller_environment);
    IdentityRegistry names;
    identity_registry_init(&names, sources.allocator);
    const IdentityName *output = NULL;
    size_t before = allocation_count;
    assert(static_key_register(NULL, environment, &names, &output) == STATIC_KEY_INVALID);
    assert(static_key_register(&terms, environment, NULL, &output) == STATIC_KEY_INVALID);
    assert(static_key_register(&terms, environment, &names, NULL) == STATIC_KEY_INVALID);
    assert(static_key_register(&terms, caller_environment, &names, &output) == STATIC_KEY_INVALID);
    assert(output == NULL && names.first == NULL && allocation_count == before);
    identity_registry_destroy(&names);
    destroy_guards();
}

/** Execute validation, allocation, overflow and collision boundaries. */
static void run(void) {
    it_refuses_open_foreign_and_malformed_owner_keys_without_publication();
    it_unwinds_every_key_encoding_allocation_and_preserves_safe_retry();
    it_checks_counted_writer_arithmetic_and_exact_field_boundaries();
    it_refuses_postorder_counter_overflow_with_owned_partial_scratch();
    it_rejects_injected_digest_collisions_without_merging_static_identities();
    it_unwinds_full_name_registration_and_retains_only_owned_name_data();
    it_refuses_invalid_naming_arguments_without_partial_names();
}
/** Return success after every partial owner has been destroyed. */
int main(void) { run(); return 0; }
