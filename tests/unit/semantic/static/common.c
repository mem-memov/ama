/** @file
 * @brief Own counted project-backed static records and exact source-name fixtures.
 */
#ifndef AMA_STATIC_TEST_COMMON
#define AMA_STATIC_TEST_COMMON
#include "../../project/common.c"
#include "semantic/static/internal.h"

/** One owner expires before its borrowed project/source definitions. */
static StaticInventory inventory;

/** Original names for independent scope records, including unequal Unicode
 * normalization spellings; comments provide bytes without declaring formals.
 */
static const char *static_scope_text = "@!\n# ...same ...result ...alias ...Same ...same-long ...ключ ...café ...cafe\xcc\x81\n";

/** Establish two exact loaded source definitions from explicit fixture texts.
 * They need not be semantically bound yet: these component tests own that stage.
 */
static void setup_static(const char *main_text, const char *other_text) {
    directory("generated/static scope");
    file("generated/static scope/main", main_text);
    file("generated/static scope/other", other_text);
    setup_project();
    ModuleRoot root = {"generated/static scope", NULL, 0, MODULE_ROOT_PROJECT};
    const char *required[] = {".other"};
    assert(project_load_required(&project, &root, 1, ".main", required, 1) == PROJECT_OK);
    assert(module_count() == 2);
    size_t before = allocation_count;
    static_inventory_init(&inventory, &project, sources.allocator);
    assert(allocation_count == before && !inventory.valid && inventory.status == STATIC_OK);
}

/** Find one exact fixture occurrence without requiring NUL-terminated source
 * bytes; all resulting spans borrow the original loaded file.
 */
static SourceSpan static_name_site(const ProjectModule *module, const char *name) {
    const SourceText *source = module->source->text;
    size_t length = source_length(source), size = strlen(name);
    assert(size <= length);
    for (size_t offset = 0; offset <= length - size; offset++) {
        if (memcmp(source_bytes(source) + offset, name, size) == 0) {
            return (SourceSpan){source, offset, offset + size};
        }
    }
    assert(false);
    return (SourceSpan){0};
}

/** Destroy scratch before every source/project borrow; assert counted teardown. */
static void destroy_static(void) {
    static_inventory_destroy(&inventory);
    assert(inventory.project == NULL && inventory.first == NULL && inventory.last == NULL);
    destroy_project();
}
#endif
