#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../common.c"
#include "source/module/catalog.h"

static void directory(const char *path) {
    int result = mkdir(path, 0700);
    assert(result == 0 || errno == EEXIST);
}

static void file(const char *path, const char *content) {
    FILE *stream = fopen(path, "wb");
    assert(stream != NULL);
    size_t length = strlen(content);
    size_t written = fwrite(content, 1, length, stream);
    int closed = fclose(stream);
    assert(written == length && closed == 0);
}

static ModuleCatalog catalog;

static void setup_catalog(void) {
    setup_test();
    module_catalog_init(&catalog, sources.allocator);
}

static void destroy_catalog(void) {
    module_catalog_destroy(&catalog);
    destroy_test();
}

static void prepare_project(void) {
    directory("generated/project");
    directory("generated/project/shop");
    file("generated/project/shop/item", "unfinished parent is allowed here");
    directory("generated/project/shop/item+");
    file("generated/project/shop/item+/验证", "@C\r\n");
    file("generated/project/shop/item+/验证.c", "/* C */");
    file("generated/project/shop/item+/验证.h", "/* header */");
    file("generated/project/shop/item+/验证.abi", "+ construct\n- destroy\n");
    directory("generated/project/shop/item+/验证+");
    file("generated/project/shop/item+/验证+/details", "@!\n");
    directory("generated/project/generated");
    file("generated/project/generated/not_a_source", "\xff");
    file("generated/project/README.md", "not a source");
}

static ModuleRoot project_root(void) {
    static const char *const exclusions[] = {"generated"};
    return (ModuleRoot){"generated/project", exclusions, 1};
}

static void it_resolves_nested_submodules_and_physical_native_companions(void) {
    prepare_project();
    setup_catalog();
    ModuleRoot root = project_root();
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_OK);
    const ModuleSource *source = NULL;
    status = module_catalog_find(&catalog, ".shop.item.验证", &source);
    assert(status == MODULE_OK);
    assert(strcmp(source_path(source->text), "generated/project/shop/item+/验证") == 0);
    assert(strcmp(source->native_c, "generated/project/shop/item+/验证.c") == 0);
    assert(strcmp(source->native_header, "generated/project/shop/item+/验证.h") == 0);
    assert(strcmp(source_path(source->native_abi), "generated/project/shop/item+/验证.abi") == 0);
    status = module_catalog_find(&catalog, ".shop.item.验证.details", &source);
    assert(status == MODULE_OK);
    status = module_catalog_find(&catalog, ".generated.not_a_source", &source);
    assert(status == MODULE_MISSING);
    status = module_catalog_find(&catalog, ".shop.item.验证.c", &source);
    assert(status == MODULE_MISSING);
    status = module_catalog_find(&catalog, ".shop..item", &source);
    assert(status == MODULE_INVALID_PATH);
    destroy_catalog();
}

static void it_rejects_duplicates_across_roots_instead_of_preferring_one(void) {
    directory("generated/first");
    directory("generated/second");
    file("generated/first/item", "@!\n");
    file("generated/second/item", "@!\n");
    ModuleRoot roots[] = {{"generated/first", NULL, 0}, {"generated/second", NULL, 0}};
    for (size_t order = 0; order < 2; order++) {
        setup_catalog();
        ModuleStatus status = module_catalog_build(&catalog, roots, 2);
        assert(status == MODULE_AMBIGUOUS);
        assert(catalog.issue.path != NULL && catalog.issue.related_path != NULL);
        const ModuleSource *source = NULL;
        status = module_catalog_find(&catalog, ".item", &source);
        assert(status == MODULE_INVALID_PATH && source == NULL);
        destroy_catalog();
        ModuleRoot temporary = roots[0];
        roots[0] = roots[1];
        roots[1] = temporary;
    }
}

static void it_rejects_orphans_hardlink_aliases_and_symlink_roots(void) {
    directory("generated/orphan");
    directory("generated/orphan/item+");
    setup_catalog();
    ModuleRoot root = {"generated/orphan", NULL, 0};
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_ORPHAN_DIRECTORY);
    destroy_catalog();
    directory("generated/aliases");
    file("generated/aliases/item", "@!\n");
    int removed = unlink("generated/aliases/other");
    assert(removed == 0 || errno == ENOENT);
    int linked = link("generated/aliases/item", "generated/aliases/other");
    assert(linked == 0);
    setup_catalog();
    root.path = "generated/aliases";
    status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_FILE_ALIAS);
    destroy_catalog();
    removed = unlink("generated/shortcut");
    assert(removed == 0 || errno == ENOENT);
    linked = symlink("first", "generated/shortcut");
    assert(linked == 0);
    setup_catalog();
    root.path = "generated/shortcut";
    status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_UNSUPPORTED_PATH);
    destroy_catalog();
}

static void it_distinguishes_case_and_canonically_equivalent_source_spellings(void) {
    directory("generated/spelling");
    const char *names[] = {"generated/spelling/Item", "generated/spelling/item",
        "generated/spelling/é", "generated/spelling/e\xcc\x81"};
    for (size_t index = 0; index < 4; index++) {
        file(names[index], "@!\n");
    }
    setup_catalog();
    ModuleRoot root = {"generated/spelling", NULL, 0};
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_OK);
    const char *logical[] = {".Item", ".item", ".é", ".e\xcc\x81"};
    for (size_t index = 0; index < 4; index++) {
        const ModuleSource *source = NULL;
        status = module_catalog_find(&catalog, logical[index], &source);
        assert(status == MODULE_OK && strcmp(source_path(source->text), names[index]) == 0);
    }
    destroy_catalog();
}

static void it_reports_the_exact_invalid_utf8_span_in_sources_and_abi(void) {
    directory("generated/invalid");
    file("generated/invalid/item", "@!\n\xff");
    ModuleRoot root = {"generated/invalid", NULL, 0};
    setup_catalog();
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_INVALID_UTF8 && catalog.issue.offset == 3);
    assert(catalog.issue.source != NULL);
    destroy_catalog();
    file("generated/invalid/item", "@C\n");
    file("generated/invalid/item.abi", "+ good\n\xed\xa0\x80");
    setup_catalog();
    status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_INVALID_UTF8 && catalog.issue.offset == 7);
    assert(strcmp(catalog.issue.path, "generated/invalid/item.abi") == 0);
    destroy_catalog();
}

static void it_rejects_aliased_directory_roots_even_when_empty(void) {
    directory("generated/empty");
    ModuleRoot roots[] = {{"generated/empty", NULL, 0}, {"generated/./empty", NULL, 0}};
    setup_catalog();
    ModuleStatus status = module_catalog_build(&catalog, roots, 2);
    assert(status == MODULE_FILE_ALIAS);
    destroy_catalog();
}

static void it_unwinds_every_catalog_allocation_failure(void) {
    setup_catalog();
    ModuleRoot root = project_root();
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_OK);
    size_t total = allocation_count;
    destroy_catalog();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_catalog();
        failure_at = failure;
        status = module_catalog_build(&catalog, &root, 1);
        assert(status == MODULE_OUT_OF_MEMORY && !catalog.built);
        destroy_catalog();
    }
}

void run(void) {
    it_resolves_nested_submodules_and_physical_native_companions();
    it_rejects_duplicates_across_roots_instead_of_preferring_one();
    it_rejects_orphans_hardlink_aliases_and_symlink_roots();
    it_distinguishes_case_and_canonically_equivalent_source_spellings();
    it_reports_the_exact_invalid_utf8_span_in_sources_and_abi();
    it_rejects_aliased_directory_roots_even_when_empty();
    it_unwinds_every_catalog_allocation_failure();
}

int main(void) {
    run();
    return EXIT_SUCCESS;
}
