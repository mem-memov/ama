/** @file
 * @brief Check final-name-driven plus placement against independent actual catalog discovery.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/mapping/physical/main.h"
/** All ancestor-presence subsets in both areas are materialized from an independent
 * expected layout. Common production discovery must recover every proposed main.
 * Fixture creation constructs a final oracle view, not a source-operation apply. */
static void it_matches_common_catalog_for_every_two_ancestor_final_topology_in_src_and_tests(void) {
    prepare();
    directory("generated/project/packages/layout"); directory("generated/project/packages/layout/pkg");
    for (unsigned area = 0; area < 2; area++) {
        for (unsigned ancestors = 0; ancestors < 4; ancestors++) {
            char relative[256], version_name[128], version_identity[128];
            int written = snprintf(version_name, sizeof(version_name), "v2.0.0-case%u", area * 4 + ancestors);
            assert(written > 0 && (size_t)written < sizeof(version_name));
            written = snprintf(version_identity, sizeof(version_identity), "/layout/pkg/%s", version_name);
            assert(written > 0 && (size_t)written < sizeof(version_identity));
            written = snprintf(relative, sizeof(relative), "packages/layout/pkg/%s", version_name);
            assert(written > 0 && (size_t)written < sizeof(relative));
            const char *root = source_arena_join(&capture.arena, capture.roots[0].path, "/", relative); assert(root);
            directory(root);
            const char *configuration = source_arena_join(&capture.arena, root, "/", "package.ini"); assert(configuration); file(configuration, "");
            const char *src = source_arena_join(&capture.arena, root, "/", "src"); assert(src); directory(src);
            const char *tests = source_arena_join(&capture.arena, root, "/", "tests"); assert(tests); directory(tests);
            const char *base = area ? tests : src;
            const char *tree_main = source_arena_join(&capture.arena, base, "/", "tree"); assert(tree_main);
            const char *tree_dir = source_arena_join(&capture.arena, base, "/", ancestors & 1 ? "tree+" : "tree"); assert(tree_dir);
            if (ancestors & 1) { file(tree_main, "@!\n"); }
            directory(tree_dir);
            const char *branch_main = source_arena_join(&capture.arena, tree_dir, "/", "branch"); assert(branch_main);
            const char *branch_dir = source_arena_join(&capture.arena, tree_dir, "/", ancestors & 2 ? "branch+" : "branch"); assert(branch_dir);
            if (ancestors & 2) { file(branch_main, "@!\n"); }
            directory(branch_dir);
            const char *check = source_arena_join(&capture.arena, branch_dir, "/", "check"); assert(check); file(check, "@!\n");
            const char *logical_area = area ? "/tests/" : "/src/";
            const char *names[3]; const char *expected[3]; size_t count = 0;
            if (ancestors & 1) { names[count] = source_arena_join(&capture.arena, version_identity, logical_area, "tree"); expected[count++] = tree_main; }
            if (ancestors & 2) { names[count] = source_arena_join(&capture.arena, version_identity, logical_area, "tree/branch"); expected[count++] = branch_main; }
            names[count] = source_arena_join(&capture.arena, version_identity, logical_area, "tree/branch/check"); expected[count++] = check;
            SourceVersionPlacement version = {version_identity, root};
            ModuleCatalog final; module_catalog_init(&final, sources.allocator);
            ModuleRoot final_root = {capture.roots[0].path, NULL, 0, MODULE_ROOT_PROJECT};
            assert(module_catalog_build(&final, &final_root, 1) == MODULE_OK);
            for (size_t i = 0; i < count; i++) {
                const char *path = NULL; const ModuleSource *actual = NULL;
                assert(source_main_physical_path(&capture.arena, &version, names, count, names[i], &path) == SOURCE_MAIN_PATH_OK);
                assert(!strcmp(path, expected[i]));
                assert(module_catalog_find(&final, names[i], &actual) == MODULE_OK && !strcmp(source_path(actual->text), path));
            }
            module_catalog_destroy(&final);
        }
    }
    finish();
}
/** Validate the full same-version set before allocation; exact ownership and
 * single-allocation refusal cover trailing-root and filesystem-root joins too. */
static void it_refuses_invalid_main_indexes_and_preserves_owned_physical_outputs(void) {
    prepare(); SourceArena work; source_arena_init(&work, sources.allocator);
    SourceVersionPlacement version = {"/a/p/v1.0.0", "/prepared/version"};
    const char *names[] = {"/a/p/v1.0.0/src/дерево", "/a/p/v1.0.0/src/дерево/child",
        "/a/p/v1.0.0/tests/дерево/child/check"};
    const char *output = "sentinel"; size_t before = allocation_count;
    assert(source_main_physical_path(NULL, &version, names, 3, names[1], &output) == SOURCE_MAIN_PATH_INVALID);
    assert(source_main_physical_path(&work, NULL, names, 3, names[1], &output) == SOURCE_MAIN_PATH_INVALID);
    assert(source_main_physical_path(&work, &version, NULL, 3, names[1], &output) == SOURCE_MAIN_PATH_INVALID);
    assert(source_main_physical_path(&work, &version, names, 3, names[1], NULL) == SOURCE_MAIN_PATH_INVALID);
    assert(source_main_physical_path(&work, &version, names, 3, "/a/p/v1.0.0/src/absent", &output) == SOURCE_MAIN_PATH_MISSING);
    assert(source_main_physical_path(&work, &version, NULL, 0, names[1], &output) == SOURCE_MAIN_PATH_MISSING);
    const char *invalid[] = {names[0], names[0]};
    assert(source_main_physical_path(&work, &version, invalid, 2, names[0], &output) == SOURCE_MAIN_PATH_INVALID);
    const char *const bad_names[] = {NULL, "//standard", "/a/p/v2.0.0/src/other", "/a/p/v1.0.0/src", "../relative"};
    for (size_t i = 0; i < sizeof(bad_names) / sizeof(bad_names[0]); i++) {
        invalid[1] = bad_names[i];
        assert(source_main_physical_path(&work, &version, invalid, 2, names[0], &output) == SOURCE_MAIN_PATH_INVALID);
    }
    version.physical = "relative";
    assert(source_main_physical_path(&work, &version, names, 3, names[1], &output) == SOURCE_MAIN_PATH_INVALID);
    version.physical = "/prepared/version"; version.identity = "/a/p/v1.0.0/src/m";
    assert(source_main_physical_path(&work, &version, names, 3, names[1], &output) == SOURCE_MAIN_PATH_INVALID);
    version.identity = "/a/p/v1.0.0";
    assert(allocation_count == before && !strcmp(output, "sentinel"));
    failure_at = allocation_count + 1;
    assert(source_main_physical_path(&work, &version, names, 3, names[1], &output) == SOURCE_MAIN_PATH_MEMORY);
    assert(!strcmp(output, "sentinel")); failure_at = 0;
    char root[] = "/prepared/version/"; version.physical = root;
    assert(source_main_physical_path(&work, &version, names, 3, names[1], &output) == SOURCE_MAIN_PATH_OK);
    root[1] = 'z'; assert(!strcmp(output, "/prepared/version/src/дерево+/child"));
    version.physical = "/";
    assert(source_main_physical_path(&work, &version, names, 3, names[2], &output) == SOURCE_MAIN_PATH_OK);
    assert(!strcmp(output, "/tests/дерево/child/check"));
    source_arena_destroy(&work); finish();
}
