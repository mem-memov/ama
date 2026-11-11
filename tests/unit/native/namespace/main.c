#include "../../source/common.c"
#include "native/namespace/namespace.h"

static void it_qualifies_local_names_with_stable_module_specific_prefixes(void) {
    setup_test();
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    QualifiedIdentity identity = {IDENTITY_NATIVE_NAMESPACE, ".商店.商品", NULL, 0, {0}};
    const IdentityName *first;
    const IdentityName *second;
    IdentityStatus status = identity_register(&registry, identity, &first);
    assert(status == IDENTITY_OK && first->category == IDENTITY_NATIVE_NAMESPACE);
    identity.module_path = ".warehouse.商品";
    status = identity_register(&registry, identity, &second);
    assert(status == IDENTITY_OK && strcmp(first->internal_name, second->internal_name) != 0);
    const SourceText *source;
    SourceStatus added = source_manager_add(&sources, "local", (const unsigned char *)"initialize", 10, &source);
    assert(added == SOURCE_OK);
    FILE *output = tmpfile();
    assert(output != NULL);
    bool emitted = native_namespace_begin(output, first);
    assert(emitted);
    emitted = native_namespace_symbol(output, first, (SourceSpan){source, 0, 10});
    assert(emitted);
    int separated = fputc('\n', output);
    assert(separated != EOF);
    emitted = native_namespace_end(output);
    assert(emitted);
    int sought = fseek(output, 0, SEEK_SET);
    assert(sought == 0);
    char bytes[512];
    size_t length = fread(bytes, 1, sizeof(bytes) - 1, output);
    assert(!ferror(output) && length < sizeof(bytes) - 1);
    bytes[length] = 0;
    assert(strstr(bytes, "#define AMA_NATIVE(local_name) ama_n_") != NULL);
    assert(strstr(bytes, "_##local_name\n") != NULL);
    assert(strstr(bytes, "_initialize\n#undef AMA_NATIVE\n") != NULL);
    int closed = fclose(output);
    assert(closed == 0);
    identity_registry_destroy(&registry);
    destroy_test();
}

static void it_rejects_non_namespace_identities_and_reports_output_failure(void) {
    setup_test();
    IdentityRegistry registry;
    identity_registry_init(&registry, sources.allocator);
    QualifiedIdentity identity = {IDENTITY_TYPE, ".type", NULL, 0, {0}};
    const IdentityName *name;
    IdentityStatus status = identity_register(&registry, identity, &name);
    assert(status == IDENTITY_OK);
    FILE *output = tmpfile();
    assert(output != NULL);
    bool emitted = native_namespace_begin(output, name);
    assert(!emitted && ftell(output) == 0);
    int closed = fclose(output);
    assert(closed == 0);
    identity.category = IDENTITY_NATIVE_NAMESPACE;
    const char *members[] = {"invalid"};
    identity.members = members;
    identity.member_count = 1;
    status = identity_register(&registry, identity, &name);
    assert(status == IDENTITY_INVALID);
    identity.members = NULL;
    identity.member_count = 0;
    status = identity_register(&registry, identity, &name);
    assert(status == IDENTITY_OK);
    output = fopen("/dev/full", "wb");
    assert(output != NULL);
    int unbuffered = setvbuf(output, NULL, _IONBF, 0);
    assert(unbuffered == 0);
    emitted = native_namespace_begin(output, name);
    assert(!emitted && ferror(output));
    fclose(output);
    identity_registry_destroy(&registry);
    destroy_test();
}

void run(void) {
    it_qualifies_local_names_with_stable_module_specific_prefixes();
    it_rejects_non_namespace_identities_and_reports_output_failure();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
