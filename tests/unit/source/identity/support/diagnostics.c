/**
 * @file
 * @brief Compose an exact unequal-key registry collision with source-owned diagnostic context.
 */
#include "diagnostics/adapters/adapters.h"

/** Digest injection exercises the exact collision branch without assuming or searching for a SHA-256 collision. */
static void it_composes_a_real_unequal_key_collision_into_both_original_declarations(void) {
    setup_test();
    const unsigned char bytes[] = "first\nsecond\n";
    const SourceText *text = NULL;
    SourceStatus added = source_manager_add(&sources, "original declarations",
        bytes, sizeof(bytes) - 1, &text);
    assert(added == SOURCE_OK);
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    QualifiedIdentity keys[] = {
        {IDENTITY_TYPE, ".товар данные", NULL, 0, {text, 0, 5}},
        {IDENTITY_TYPE, "..товар данные", NULL, 0, {text, 6, 12}}
    };
    IdentityEncoding encodings[2];
    for (size_t index = 0; index < 2; index++) {
        IdentityStatus status = identity_encode(&registry.arena, keys[index], &encodings[index]);
        assert(status == IDENTITY_OK);
    }
    const unsigned char digest[32] = {0};
    const IdentityName *previous = NULL;
    IdentityStatus status = identity_insert(&registry, encodings[0], digest, &previous);
    assert(status == IDENTITY_OK);
    const IdentityName *unchanged = previous;
    status = identity_insert(&registry, encodings[1], digest, &unchanged);
    assert(status == IDENTITY_COLLISION);
    assert(unchanged == previous && registry.conflicting == previous);
    Diagnostic diagnostic;
    DiagnosticRelated note;
    size_t allocations = allocation_count;
    bool adapted = diagnostic_from_identity_collision(registry.conflicting, encodings[1].source_name,
        keys[1].declaration, &note, &diagnostic);
    assert(adapted);
    assert(allocation_count == allocations && diagnostic_valid(&diagnostic));
    assert(diagnostic.code == DIAGNOSTIC_IDENTITY_COLLISION);
    assert(strcmp(diagnostic.subject, "..товар данные") == 0);
    assert(strcmp(note.name, ".товар данные") == 0);
    assert(diagnostic.primary.span.source == text && diagnostic.primary.span.start == 6);
    assert(note.location.span.source == text && note.location.span.start == 0);
    assert(diagnostic.cause != NULL && diagnostic.related_count == 1);
    identity_registry_destroy(&registry);
    destroy_test();
}
