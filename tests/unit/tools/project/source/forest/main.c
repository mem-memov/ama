/** @file
 * @brief Check complete mutation trees, exact physical-domain accounting and every allocation refusal.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#define _XOPEN_SOURCE 700
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/transaction/forest/forest.h"
static SourceArena arena;
static ModuleRoot root;
static SourceCatalogImageEntry original_rows[5], final_rows[7];
static SourceCatalogImage original, final;
/** Independent exact source bytes include an embedded zero. */
static const SourceText *text(const char *path, const unsigned char *bytes, size_t count) {
    const SourceText *output = NULL;
    assert(source_manager_add(&sources, path, bytes, count, &output) == SOURCE_OK); return output;
}
/** Construct a complete finite tree whose explicit observations have unique identities. */
static void begin(void) {
    setup_test(); source_arena_init(&arena, sources.allocator);
    root = (ModuleRoot){.path = "/tree", .kind = MODULE_ROOT_PROJECT};
    const char *paths[] = {"/tree", "/tree/from", "/tree/from/a", "/tree/from/empty", "/tree/keep"};
    for (size_t i = 0; i < 5; i++) {
        bool directory = i != 2 && i != 4;
        original_rows[i] = (SourceCatalogImageEntry){.path = paths[i], .metadata = {.st_dev = 1, .st_ino = (ino_t)(i + 1), .st_mode = (directory ? S_IFDIR : S_IFREG) | 0700},
            .metadata_captured = true, .children_complete = directory};
        if (!directory) { original_rows[i].content = text(paths[i], (const unsigned char *)"a\0b", 3); }
        final_rows[i] = original_rows[i];
    }
}
/** Validate both input images rather than passing fabricated valid flags. */
static void prepare(size_t count) {
    SourceImageIssue issue;
    assert(source_catalog_image_prepare(&original, original_rows, 5, &root, 1, (SourceImageBounds){0}, &issue) == SOURCE_IMAGE_OK);
    assert(source_catalog_image_prepare(&final, final_rows, count, &root, 1, (SourceImageBounds){0}, &issue) == SOURCE_IMAGE_OK);
}
/** Retire forest arrays before text/image storage. */
static void end(void) { source_arena_destroy(&arena); destroy_test(); }
/** Exact roots cover independent file edits or one complete parent replacement;
 * all unchanged descendant material remains in its recoverable parent tree. */
static void it_partitions_changed_files_and_complete_replaced_subtrees(void) {
    begin(); final_rows[2].content = text("/tree/from/a", (const unsigned char *)"changed", 7);
    final_rows[5] = (SourceCatalogImageEntry){.path = "/tree/from/b", .metadata = {.st_dev = 0, .st_ino = 11, .st_mode = S_IFREG | 0600}, .metadata_captured = true,
        .content = text("/tree/from/b", (const unsigned char *)"new", 3)};
    prepare(6); SourceMutationForest forest; SourceForestIssue issue;
    assert(source_mutation_forest(&arena, &original, &final, &forest, &issue) == SOURCE_FOREST_OK && forest.count == 2);
    assert(!strcmp(forest.roots[0].path, "/tree/from/a") && !strcmp(forest.roots[1].path, "/tree/from/b"));
    assert(forest.roots[0].before_count == 1 && forest.roots[1].before_count == 0 && forest.roots[1].after_count == 1);
    final_rows[1].metadata.st_ino = 100;
    prepare(6);
    assert(source_mutation_forest(&arena, &original, &final, &forest, &issue) == SOURCE_FOREST_OK && forest.count == 1);
    assert(!strcmp(forest.roots[0].path, "/tree/from") && !strcmp(forest.roots[0].parent, "/tree") && !strcmp(forest.roots[0].name, "from"));
    assert(forest.roots[0].before_count == 3 && forest.roots[0].after_count == 4);
    assert(!strcmp(forest.roots[0].after_entries[3]->path, "/tree/from/empty"));
    assert(!issue.path && !issue.entry); end();
}
/** Whole deletion, empty directories, additions and byte/order-independent no-ops
 * have exact independently expected roots. */
static void it_retains_exact_delete_add_and_empty_tree_material_and_ignores_timestamps(void) {
    begin(); final_rows[1] = original_rows[4]; prepare(2);
    SourceMutationForest forest; SourceForestIssue issue;
    assert(source_mutation_forest(&arena, &original, &final, &forest, &issue) == SOURCE_FOREST_OK && forest.count == 1);
    assert(!strcmp(forest.roots[0].path, "/tree/from") && forest.roots[0].before_count == 3 && !forest.roots[0].after_count && !forest.roots[0].after);
    for (size_t i = 0; i < 5; i++) { final_rows[4 - i] = original_rows[i]; final_rows[4 - i].metadata.st_mtime++; }
    prepare(5);
    assert(source_mutation_forest(&arena, &original, &final, &forest, &issue) == SOURCE_FOREST_OK && !forest.count && !forest.roots);
    final_rows[5] = (SourceCatalogImageEntry){.path = "/tree/added", .metadata = {.st_mode = S_IFDIR | 0700, .st_ino = 80}, .metadata_captured = true, .children_complete = true}; prepare(6);
    assert(source_mutation_forest(&arena, &original, &final, &forest, &issue) == SOURCE_FOREST_OK && forest.count == 1);
    assert(!forest.roots[0].before && !forest.roots[0].before_count && forest.roots[0].after_count == 1); end();
}
/** Unknown bytes, incomplete parents, unsupported kinds and changed observation
 * roots never become assumed deletions or accepted partial mutation forests. */
static void it_refuses_unobserved_mutation_material_and_domain_changes(void) {
    for (unsigned profile = 0; profile < 5; profile++) {
        begin(); final_rows[2].metadata.st_ino = 90;
        if (profile == 0) { original_rows[2].content = NULL; }
        if (profile == 1) { final_rows[2].content = NULL; }
        if (profile == 2) { original_rows[1].children_complete = false; }
        if (profile == 3) { final_rows[2].content = NULL; final_rows[2].metadata.st_mode = S_IFIFO | 0600; }
        if (profile == 4) { final_rows[0].metadata.st_ino = 91; }
        prepare(5); SourceMutationForest sentinel = {.count = 123}, forest = sentinel; SourceForestIssue issue;
        SourceForestStatus status = source_mutation_forest(&arena, &original, &final, &forest, &issue);
        assert(status == (profile == 2 || profile == 4 ? SOURCE_FOREST_PARENT : profile == 3 ? SOURCE_FOREST_UNSUPPORTED : SOURCE_FOREST_UNOBSERVED));
        assert(forest.count == sentinel.count && forest.roots == sentinel.roots); end();
    }
    begin(); prepare(5); ModuleRoot foreign_root = root; foreign_root.kind = MODULE_ROOT_STANDARD;
    SourceImageIssue shape; assert(source_catalog_image_prepare(&final, final_rows, 5, &foreign_root, 1, (SourceImageBounds){0}, &shape) == SOURCE_IMAGE_OK);
    SourceMutationForest forest = {.count = 123}; SourceForestIssue issue;
    assert(source_mutation_forest(&arena, &original, &final, &forest, &issue) == SOURCE_FOREST_OK && !forest.count);
    const char *bounds[] = {"/tree", "/tree/from"};
    assert(source_catalog_image_prepare(&final, final_rows, 5, &foreign_root, 1,
        (SourceImageBounds){bounds, 2}, &shape) == SOURCE_IMAGE_OK);
    forest = (SourceMutationForest){.count = 123};
    assert(source_mutation_forest(&arena, &original, &final, &forest, &issue) == SOURCE_FOREST_INVALID && forest.count == 123);
    end();
}
/** Every failed arena allocation leaves output unchanged and every prefix owned. */
static void it_retires_every_mutation_forest_allocation_failure(void) {
    begin(); final_rows[1].metadata.st_ino = 99; final_rows[4].metadata.st_ino = 100; prepare(5);
    size_t total = 0, live = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceArena attempt; source_arena_init(&attempt, sources.allocator);
        SourceMutationForest forest = {.count = 123}; SourceForestIssue issue; size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceForestStatus status = source_mutation_forest(&attempt, &original, &final, &forest, &issue);
        if (!failure) { assert(status == SOURCE_FOREST_OK && forest.count == 2); total = allocation_count - before; assert(total); }
        else { assert(status == SOURCE_FOREST_MEMORY && forest.count == 123 && !forest.roots); }
        failure_at = 0; source_arena_destroy(&attempt); assert(live_allocations == live);
    }
    end();
}
/** Explicit runner keeps delta, complete material, refusal and allocation laws visible. */
void run(void) {
    it_partitions_changed_files_and_complete_replaced_subtrees();
    it_retains_exact_delete_add_and_empty_tree_material_and_ignores_timestamps();
    it_refuses_unobserved_mutation_material_and_domain_changes();
    it_retires_every_mutation_forest_allocation_failure();
}
/** Return only after all pure image and text owners retire. */
int main(void) { run(); return 0; }
