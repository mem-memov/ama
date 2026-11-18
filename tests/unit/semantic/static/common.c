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
 * A literal macro introduces no unused storage in narrower fixture consumers.
 */
#define STATIC_SCOPE_ROOT_DIRECTORY "generated/static scope"
#define STATIC_SCOPE_SOURCE_TEXT "@!\n# ...same ...result ...alias ...Same ...same-long ...ключ ...café ...cafe\xcc\x81\n"

/** Load explicit fixture sources through caller-supplied root families. Roots
 * include STATIC_SCOPE_ROOT_DIRECTORY as project storage; standard/import roots
 * may extend the actual required closure without a fabricated module count.
 */
static void setup_static_with_roots(const char *main_text, const char *other_text,
    const ModuleRoot *roots, size_t root_count) {
    directory(STATIC_SCOPE_ROOT_DIRECTORY);
    file("generated/static scope/main", main_text);
    file("generated/static scope/other", other_text);
    setup_project();
    const char *required[] = {".other"};
    ProjectStatus status = project_load_required(&project, roots, root_count, ".main", required, 1);
    if (status != PROJECT_OK) {
        assert(diagnostic_valid(&project.error));
        fprintf(stderr, "Static fixture project failure: %s at bytes %zu..%zu\n",
            project.error.message, project.error.primary.span.start, project.error.primary.span.end);
    }
    assert(status == PROJECT_OK);
    assert(project.entry != NULL && project_find(&project, ".other") != NULL);
    size_t before = allocation_count;
    static_inventory_init(&inventory, &project, sources.allocator);
    assert(allocation_count == before && !inventory.valid && inventory.status == STATIC_OK);
}

/** Preserve the original exact two-definition domain for tests that need no
 * standard/import sources. The generalized loader also serves real list roots.
 */
static inline void setup_static(const char *main_text, const char *other_text) {
    ModuleRoot root = {STATIC_SCOPE_ROOT_DIRECTORY, NULL, 0, MODULE_ROOT_PROJECT};
    setup_static_with_roots(main_text, other_text, &root, 1);
    assert(module_count() == 2);
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
