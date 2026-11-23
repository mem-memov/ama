/** @file
 * @brief Verify actual durable private trees and exhaustive preparation-failure cleanup.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#define _XOPEN_SOURCE 700
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/transaction/tree/tree.h"
static SourceArena arena;
static ModuleRoot root;
static SourceCatalogImageEntry before_rows[5], after_rows[5];
static SourceCatalogImage before_image, after_image;
static SourceMutationForest forest;
static int parent;
static char directory[64];
/** Real source observations feed the production forest; its material is not fabricated. */
static void begin(void) {
    setup_test();
    source_arena_init(&arena, sources.allocator);
    assert(mkdir("generated", 0700) == 0 || errno == EEXIST);
    strcpy(directory, "generated/private-tree-XXXXXX");
    assert(mkdtemp(directory));
    parent = open(directory, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    assert(parent >= 0);
    root = (ModuleRoot){.path = "/tree", .kind = MODULE_ROOT_PROJECT};
    const char *paths[] = {"/tree", "/tree/from", "/tree/from/a", "/tree/from/empty", "/tree/from/z"};
    unsigned char binary[70003];
    for (size_t i = 0; i < sizeof(binary); i++) { binary[i] = (unsigned char)(i % 256); }
    for (size_t i = 0; i < 5; i++) {
        bool directory_row = i < 2 || i == 3;
        before_rows[i] = (SourceCatalogImageEntry){.path = paths[i],
            .metadata = {.st_dev = 1, .st_ino = (ino_t)(i + 1), .st_mode = directory_row ? S_IFDIR | 0500 : S_IFREG | 0640},
            .metadata_captured = true, .children_complete = directory_row};
        if (!directory_row) {
            assert(source_manager_add(&sources, paths[i], binary, i == 2 ? sizeof(binary) : 0, &before_rows[i].content) == SOURCE_OK);
        }
        after_rows[i] = before_rows[i];
    }
    after_rows[1].metadata.st_ino = 100;
    SourceImageIssue shape;
    assert(source_catalog_image_prepare(&before_image, before_rows, 5, &root, 1, &shape) == SOURCE_IMAGE_OK);
    assert(source_catalog_image_prepare(&after_image, after_rows, 5, &root, 1, &shape) == SOURCE_IMAGE_OK);
    SourceForestIssue issue;
    assert(source_mutation_forest(&arena, &before_image, &after_image, &forest, &issue) == SOURCE_FOREST_OK);
    assert(forest.count == 1 && forest.roots[0].after_count == 4);
}
/** Refuse hidden material leaks rather than recursively deleting unknown paths. */
static void end(void) {
    assert(close(parent) == 0 && rmdir(directory) == 0);
    source_arena_destroy(&arena);
    destroy_test();
}
/** Count live descriptors using a real OS view, excluding the temporary scan owner. */
static size_t descriptors(void) {
    DIR *directory = opendir("/proc/self/fd");
    assert(directory);
    size_t count = 0;
    while (readdir(directory)) { count++; }
    assert(closedir(directory) == 0);
    return count;
}
/** A ready tree preserves binary/empty files, empty dirs and exact modes; repeated
 * verification and cleanup remain valid and close every retained descriptor. */
static void it_materializes_actual_forest_trees_and_retires_all_descriptors(void) {
    begin();
    size_t open_before = descriptors();
    SourceMaterialTree tree;
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_prepare(&tree, parent, "copy", forest.roots[0].after_entries,
        forest.roots[0].after_count, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_OK && tree.ready);
    assert(source_material_tree_verify(&tree) && source_material_tree_verify(&tree));
    struct stat metadata;
    assert(fstatat(parent, "copy", &metadata, AT_SYMLINK_NOFOLLOW) == 0 && (metadata.st_mode & 07777) == 0500);
    assert(fstatat(parent, "copy/a", &metadata, AT_SYMLINK_NOFOLLOW) == 0 && metadata.st_size == 70003 && (metadata.st_mode & 07777) == 0640);
    assert(fstatat(parent, "copy/z", &metadata, AT_SYMLINK_NOFOLLOW) == 0 && !metadata.st_size);
    assert(source_material_tree_prepare(&tree, parent, "second", forest.roots[0].after_entries,
        forest.roots[0].after_count, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_INVALID && tree.ready);
    assert(source_material_tree_cleanup(&tree) && source_material_tree_cleanup(&tree) && !tree.ready);
    assert(source_material_tree_destroy(&tree) == 0 && descriptors() == open_before);
    /* A single regular root is valid; no implicit directory is introduced. */
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_prepare(&tree, parent, "file", forest.roots[0].after_entries + 1, 1,
        (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_OK);
    assert(source_material_tree_cleanup(&tree) && source_material_tree_destroy(&tree) == 0);
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_prepare(&tree, parent, "retained", forest.roots[0].after_entries + 1, 1,
        (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_OK);
    assert(source_material_tree_destroy(&tree) == 0);
    assert(fstatat(parent, "retained", &metadata, 0) == 0 && metadata.st_size == 70003);
    assert(unlinkat(parent, "retained", 0) == 0);
    end();
}
/** Complete-prefix cleanup rejects both unknown children and replaced own entries
 * before any unlink, even when the replacement has identical bytes. */
static void it_preserves_foreign_children_replacements_and_existing_collisions(void) {
    begin();
    SourceMaterialTree tree;
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_prepare(&tree, parent, "copy", forest.roots[0].after_entries,
        forest.roots[0].after_count, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_OK);
    int copy = openat(parent, "copy", O_RDONLY | O_DIRECTORY);
    assert(copy >= 0 && fchmod(copy, 0700) == 0);
    /* Restore expected mode before checking: only the extra name differs. */
    int extra = openat(copy, "foreign", O_WRONLY | O_CREAT | O_EXCL, 0600);
    assert(extra >= 0 && close(extra) == 0 && fchmod(copy, 0500) == 0);
    assert(!source_material_tree_cleanup(&tree) && tree.status == SOURCE_MATERIAL_TREE_CHANGED && !tree.ready);
    struct stat metadata;
    assert(fstatat(copy, "a", &metadata, 0) == 0 && fstatat(copy, "foreign", &metadata, 0) == 0);
    assert(fchmod(copy, 0700) == 0 && unlinkat(copy, "foreign", 0) == 0);
    assert(renameat(copy, "z", parent, "saved-z") == 0);
    extra = openat(copy, "z", O_WRONLY | O_CREAT | O_EXCL, 0640);
    assert(extra >= 0 && close(extra) == 0 && fchmod(copy, 0500) == 0);
    assert(!source_material_tree_cleanup(&tree) && tree.status == SOURCE_MATERIAL_TREE_CHANGED);
    assert(fchmod(copy, 0700) == 0 && unlinkat(copy, "z", 0) == 0 && renameat(parent, "saved-z", copy, "z") == 0 && fchmod(copy, 0500) == 0);
    int changed = openat(copy, "a", O_RDWR);
    unsigned char wrong = 17, original = 0;
    assert(changed >= 0 && pwrite(changed, &wrong, 1, 0) == 1);
    assert(!source_material_tree_verify(&tree) && tree.status == SOURCE_MATERIAL_TREE_CHANGED);
    assert(pwrite(changed, &original, 1, 0) == 1 && close(changed) == 0);
    assert(source_material_tree_cleanup(&tree) && close(copy) == 0 && source_material_tree_destroy(&tree) == 0);
    assert(symlinkat("missing", parent, "collision") == 0);
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_prepare(&tree, parent, "collision", forest.roots[0].after_entries,
        forest.roots[0].after_count, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_IO);
    assert(source_material_tree_cleanup(&tree) && source_material_tree_destroy(&tree) == 0);
    assert(fstatat(parent, "collision", &metadata, AT_SYMLINK_NOFOLLOW) == 0 && S_ISLNK(metadata.st_mode));
    assert(unlinkat(parent, "collision", 0) == 0);
    end();
}
/** Cancel one exact preparation operation, including a second binary write. */
typedef struct { size_t count, fail; } Failure;
/** Deterministic boundaries avoid timing assumptions and never cancel cleanup. */
static bool permit(void *context, SourceTreeOperation operation, const char *path) {
    Failure *failure = context;
    (void)operation;
    assert(path);
    return ++failure->count != failure->fail;
}
/** Every operation/allocation prefix remains explicitly cleanable and leak free. */
static void it_cleans_every_private_preparation_failure_without_source_changes(void) {
    begin();
    size_t calls = 0, allocations = 0, live = live_allocations, open_before = descriptors();
    for (size_t ordinal = 0; !ordinal || ordinal <= calls + allocations; ordinal++) {
        SourceMaterialTree tree;
        source_material_tree_init(&tree, sources.allocator);
        Failure failure = {.fail = ordinal <= calls ? ordinal : 0};
        size_t allocated_before = allocation_count;
        if (ordinal > calls) { failure_at = allocated_before + ordinal - calls; }
        SourceMaterialTreeStatus status = source_material_tree_prepare(&tree, parent, "copy", forest.roots[0].after_entries,
            forest.roots[0].after_count, (SourceTreeControl){&failure, permit});
        failure_at = 0;
        if (!ordinal) {
            assert(status == SOURCE_MATERIAL_TREE_OK);
            calls = failure.count;
            allocations = allocation_count - allocated_before;
            assert(calls && allocations);
        } else {
            assert(status == (ordinal <= calls ? SOURCE_MATERIAL_TREE_CANCELLED : SOURCE_MATERIAL_TREE_MEMORY) && !tree.ready);
        }
        assert(source_material_tree_cleanup(&tree) && source_material_tree_destroy(&tree) == 0);
        assert(live_allocations == live && descriptors() == open_before);
    }
    SourceMaterialTree tree;
    source_material_tree_init(&tree, sources.allocator);
    assert(source_material_tree_prepare(&tree, parent, "../escape", forest.roots[0].after_entries,
        forest.roots[0].after_count, (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_INVALID);
    assert(source_material_tree_cleanup(&tree) && source_material_tree_destroy(&tree) == 0);
    for (unsigned profile = 0; profile < 5; profile++) {
        SourceCatalogImageEntry malformed = *forest.roots[0].after_entries[1];
        if (profile == 0) { malformed.path = "/tree/from/../outside"; }
        if (profile == 1) { malformed.metadata_captured = false; }
        if (profile == 2) { malformed.content = NULL; }
        if (profile == 3) { malformed.path = "/tree/from/missing/child"; }
        if (profile == 4) { malformed.path = forest.roots[0].after_entries[0]->path; }
        const SourceCatalogImageEntry *invalid[] = {forest.roots[0].after_entries[0], &malformed};
        source_material_tree_init(&tree, sources.allocator);
        assert(source_material_tree_prepare(&tree, parent, "invalid", invalid, 2,
            (SourceTreeControl){0}) == SOURCE_MATERIAL_TREE_INVALID);
        assert(source_material_tree_cleanup(&tree) && source_material_tree_destroy(&tree) == 0);
    }
    end();
}
/** Explicit actual-filesystem, ownership, collision and exhaustive failure coverage. */
void run(void) {
    it_materializes_actual_forest_trees_and_retires_all_descriptors();
    it_preserves_foreign_children_replacements_and_existing_collisions();
    it_cleans_every_private_preparation_failure_without_source_changes();
}
/** Exit only after all staged material and descriptor owners retire. */
int main(void) { run(); return 0; }
