/** @file
 * @brief Reject full-key disagreement even when independent components have equal candidate hashes.
 */
/** Inject equal candidate names while preserving different complete ABI keys and valid typed models. */
static void it_rejects_cross_component_hash_collisions_in_headers_and_bootstrap(void) {
    prepare_project(); setup_test();
    LibraryBuild first, second;
    component(&first, "example.first"); component(&second, "example.other");
    IdentityName collision = *second.identity;
    /* This boundary injection models a SHA candidate collision between two
     * independent registries without searching for a cryptographic collision.
     * Only candidate bytes/names agree; the source keys stay exact and unequal.
     */
    memcpy(collision.digest, first.identity->digest, sizeof(collision.digest));
    memcpy(collision.internal_name, first.identity->internal_name, sizeof(collision.internal_name));
    const IdentityName *original = second.identity;
    second.identity = &collision;
    assert(first.identity->encoding_length == second.identity->encoding_length);
    assert(memcmp(first.identity->encoding, second.identity->encoding, first.identity->encoding_length) != 0);
    header(&first, "generated/first.h"); header(&second, "generated/collision.h");
    file("generated/collision-include.c", "#include \"first.h\"\n#include \"collision.h\"\n");
    char diagnostic[32768];
    bool compiled = compile("generated/collision-include.c", NULL, "generated/collision.o", false, NULL, diagnostic, sizeof(diagnostic));
    assert(!compiled && strstr(diagnostic, "Conflicting complete Ama component keys") != NULL);
    FILE *receiver = fopen("generated/collision-receiver.c", "wb"); assert(receiver != NULL);
    int written = fputs("#include \"first.h\"\n", receiver); assert(written != EOF);
    bool emitted = library_abi_write(&first, receiver); assert(emitted);
    int closed = fclose(receiver); assert(closed == 0);
    char source[2048];
    int length = snprintf(source, sizeof(source),
        "#include \"collision.h\"\n#include <assert.h>\nint main(void){assert(%s_abi_check()==4);return 0;}\n", first.identity->internal_name);
    assert(length > 0 && (size_t)length < sizeof(source));
    file("generated/collision-consumer.c", source);
    compiled = compile("generated/collision-consumer.c", "generated/collision-receiver.c", "generated/collision-check", true, NULL, diagnostic, sizeof(diagnostic));
    if (!compiled) { fprintf(stderr, "%s", diagnostic); }
    assert(compiled);
    execute("generated/collision-check");
    second.identity = original;
    library_build_destroy(&second); library_build_destroy(&first); destroy_test();
}
