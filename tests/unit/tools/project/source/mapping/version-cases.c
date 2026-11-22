/** @file
 * @brief Compare finite copy-version propagation with independent reachability semantics.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/mapping/versions/plan.h"
#include "../../../../../../tools/project/source/mapping/physical/main.h"
/** Build a fixed pool of actual catalog originals; the demand solver itself
 * deliberately owns no source discovery or associated-test ownership policy. */
static void version_units(SourceVersionUnit units[3]) {
    units[0] = (SourceVersionUnit){original("/vendor/pkg/v1.0.0/src/parent"), NULL};
    units[1] = (SourceVersionUnit){original("/vendor/pkg/v1.0.0/src/parent/child"), NULL};
    units[2] = (SourceVersionUnit){original("/vendor/pkg/v1.0.0/src/parentish"), NULL};
}
/** Floyd-Warshall reachability is independent of production's changing-label
 * passes. Incoming edges to explicitly placed targets do not transmit demands. */
static SourceVersionStatus version_oracle(const unsigned seeds[3], unsigned graph, unsigned expected[3]) {
    bool reaches[3][3];
    for (size_t i = 0; i < 3; i++) {
        for (size_t j = 0; j < 3; j++) {
            reaches[i][j] = i == j || (!seeds[j] && (graph & (1u << (i * 3 + j))));
        }
    }
    for (size_t k = 0; k < 3; k++) {
        for (size_t i = 0; i < 3; i++) {
            for (size_t j = 0; j < 3; j++) { reaches[i][j] |= reaches[i][k] && reaches[k][j]; }
        }
    }
    bool unknown = false;
    for (size_t j = 0; j < 3; j++) {
        unsigned mask = 0;
        for (size_t i = 0; i < 3; i++) { if (seeds[i] && reaches[i][j]) { mask |= 1u << seeds[i]; } }
        if (mask == 6) { return SOURCE_VERSION_CONFLICT; }
        expected[j] = mask == 2 ? 1 : mask == 4 ? 2 : 0;
        unknown |= !mask;
    }
    return unknown ? SOURCE_VERSION_UNRESOLVED : SOURCE_VERSION_OK;
}
/** Exhaust every three-node directed graph and assignment of zero/two distinct
 * fixed destinations in both edge orders, including cycles and disconnected SCCs. */
static void it_matches_all_three_unit_version_demand_graphs_and_seed_assignments(void) {
    prepare(); SourceVersionUnit units[3]; version_units(units);
    const char *const versions[] = {NULL, "/target/a/v1.0.0", "/target/b/v2.0.0"};
    for (unsigned seed_set = 0; seed_set < 27; seed_set++) {
        unsigned seeds[3], digits = seed_set;
        for (size_t i = 0; i < 3; i++) { seeds[i] = digits % 3; digits /= 3; units[i].fixed_version = versions[seeds[i]]; }
        for (unsigned graph = 0; graph < 512; graph++) {
            unsigned expected[3] = {0}; SourceVersionStatus wanted = version_oracle(seeds, graph, expected);
            SourceVersionFlow flows[9]; size_t count = 0;
            for (size_t i = 0; i < 3; i++) {
                for (size_t j = 0; j < 3; j++) {
                    if (graph & (1u << (i * 3 + j))) { flows[count++] = (SourceVersionFlow){i, j}; }
                }
            }
            for (unsigned order = 0; order < 2; order++) {
                SourceArena work; source_arena_init(&work, sources.allocator);
                SourceVersionAssignment sentinel = {0}; const SourceVersionAssignment *output = &sentinel;
                SourceVersionIssue issue = {0};
                SourceVersionStatus status = source_version_plan(&work, units, 3, flows, count, &output, &issue);
                assert(status == wanted);
                if (status == SOURCE_VERSION_OK) {
                    for (size_t i = 0; i < 3; i++) {
                        assert(output[i].original == units[i].original && !strcmp(output[i].version, versions[expected[i]]));
                    }
                } else {
                    assert(output == &sentinel);
                    if (status == SOURCE_VERSION_CONFLICT) {
                        assert(issue.unit < 3 && issue.flow < count && !units[issue.unit].fixed_version);
                        assert(issue.first && issue.second && strcmp(issue.first, issue.second));
                    } else { assert(issue.unit < 3 && !expected[issue.unit]); }
                }
                source_arena_destroy(&work);
                for (size_t i = 0; i < count / 2; i++) {
                    SourceVersionFlow swap = flows[i]; flows[i] = flows[count - i - 1]; flows[count - i - 1] = swap;
                }
            }
        }
    }
    finish();
}
/** Every published version is owned; fail every measured work/result/string
 * allocation without publishing a prefix, then verify complete counted teardown. */
static void it_owns_version_assignments_and_retires_every_allocation_failure(void) {
    prepare(); SourceVersionUnit units[3]; version_units(units);
    char fixed[] = "/target/a/v1.0.0"; units[0].fixed_version = fixed;
    SourceVersionFlow flows[] = {{0, 1}, {1, 2}, {2, 1}, {0, 1}};
    size_t allocations = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= allocations; failure++) {
        SourceArena work; source_arena_init(&work, sources.allocator);
        SourceVersionAssignment sentinel = {0}; const SourceVersionAssignment *output = &sentinel;
        SourceVersionIssue issue = {0}; size_t before = allocation_count;
        if (failure) { failure_at = before + failure; }
        SourceVersionStatus status = source_version_plan(&work, units, 3, flows, 4, &output, &issue);
        if (!failure) {
            assert(status == SOURCE_VERSION_OK); allocations = allocation_count - before; assert(allocations);
            fixed[1] = 'z';
            for (size_t i = 0; i < 3; i++) { assert(!strcmp(output[i].version, "/target/a/v1.0.0")); }
            fixed[1] = 't';
        } else { assert(status == SOURCE_VERSION_MEMORY && output == &sentinel); }
        failure_at = 0; source_arena_destroy(&work); assert(live_allocations == baseline);
    }
    finish();
}
/** Reject every malformed endpoint/edge/owner before allocation and allow the
 * exact empty graph; repeated logical identities cannot bypass uniqueness. */
static void it_refuses_invalid_version_demand_graphs_without_allocating(void) {
    prepare(); SourceVersionUnit units[3]; version_units(units);
    SourceArena work; source_arena_init(&work, sources.allocator);
    SourceVersionAssignment sentinel = {0}; const SourceVersionAssignment *output = &sentinel;
    SourceVersionIssue issue = {0}; SourceVersionFlow edge = {0, 3};
    size_t before = allocation_count;
    assert(source_version_plan(&work, NULL, 3, NULL, 0, &output, &issue) == SOURCE_VERSION_INVALID);
    assert(source_version_plan(&work, units, 3, NULL, 1, &output, &issue) == SOURCE_VERSION_INVALID);
    assert(source_version_plan(&work, units, 3, &edge, 1, &output, &issue) == SOURCE_VERSION_INVALID);
    edge = (SourceVersionFlow){3, 0};
    assert(source_version_plan(&work, units, 3, &edge, 1, &output, &issue) == SOURCE_VERSION_INVALID);
    units[1].original = units[0].original;
    assert(source_version_plan(&work, units, 3, NULL, 0, &output, &issue) == SOURCE_VERSION_INVALID);
    ModuleSource duplicate = *units[0].original; units[1].original = &duplicate;
    assert(source_version_plan(&work, units, 3, NULL, 0, &output, &issue) == SOURCE_VERSION_INVALID);
    version_units(units); units[1].original = NULL;
    assert(source_version_plan(&work, units, 3, NULL, 0, &output, &issue) == SOURCE_VERSION_INVALID);
    version_units(units); ModuleSource standard = {.logical_path = "//integer"}; units[1].original = &standard;
    assert(source_version_plan(&work, units, 3, NULL, 0, &output, &issue) == SOURCE_VERSION_INVALID);
    version_units(units); units[0].fixed_version = "/target/a/v1.0.0/src/m";
    assert(source_version_plan(&work, units, 3, NULL, 0, &output, &issue) == SOURCE_VERSION_INVALID);
    units[0].fixed_version = "//standard";
    assert(source_version_plan(&work, units, 3, NULL, 0, &output, &issue) == SOURCE_VERSION_INVALID);
    assert(source_version_plan(NULL, NULL, 0, NULL, 0, &output, &issue) == SOURCE_VERSION_INVALID);
    assert(source_version_plan(&work, NULL, 0, NULL, 0, NULL, &issue) == SOURCE_VERSION_INVALID);
    assert(source_version_plan(&work, NULL, 0, NULL, 0, &output, NULL) == SOURCE_VERSION_INVALID);
    assert(output == &sentinel && allocation_count == before);
    assert(source_version_plan(&work, NULL, 0, NULL, 0, &output, &issue) == SOURCE_VERSION_OK && !output);
    assert(allocation_count == before);
    source_arena_destroy(&work); finish();
}

/** Version propagation feeds complete additional names and the existing clone
 * relation while preserving nested suffixes; explicit worker placement stays put. */
static void it_composes_version_demands_additional_suffixes_and_injective_clone_proposals(void) {
    prepare(); SourceVersionUnit units[3]; version_units(units);
    units[0].fixed_version = "/target/a/v2.0.0";
    SourceVersionFlow flows[] = {{0, 1}, {1, 2}, {2, 1}};
    SourceArena work; source_arena_init(&work, sources.allocator);
    const SourceVersionAssignment *assignments = NULL; SourceVersionIssue issue = {0};
    assert(source_version_plan(&work, units, 3, flows, 3, &assignments, &issue) == SOURCE_VERSION_OK);
    source_clone_map_init(&map, sources.allocator, &capture); const SourceCloneMapping *row = NULL;
    assert(source_clone_map_add(&map, units[0].original, "/prepared/src/renamed",
        "/target/a/v2.0.0/src/renamed", &row) == SOURCE_CLONE_OK);
    const char *names[3] = {"/target/a/v2.0.0/src/renamed", NULL, NULL};
    assert(source_additional_module_identity(&work, &assignments[1], &names[1]) == SOURCE_OK);
    assert(!strcmp(names[1], "/target/a/v2.0.0/src/parent/child"));
    assert(source_additional_module_identity(&work, &assignments[2], &names[2]) == SOURCE_OK);
    assert(!strcmp(names[2], "/target/a/v2.0.0/src/parentish"));
    SourceVersionPlacement version = {"/target/a/v2.0.0", "/prepared"};
    const char *path = NULL;
    assert(source_main_physical_path(&work, &version, names, 3, names[1], &path) == SOURCE_MAIN_PATH_OK);
    assert(!strcmp(path, "/prepared/src/parent/child"));
    assert(source_clone_map_add(&map, units[1].original, path, names[1], &row) == SOURCE_CLONE_OK);
    assert(source_main_physical_path(&work, &version, names, 3, names[2], &path) == SOURCE_MAIN_PATH_OK);
    assert(!strcmp(path, "/prepared/src/parentish"));
    assert(source_clone_map_add(&map, units[2].original, path, names[2], &row) == SOURCE_CLONE_OK);
    assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
    source_arena_destroy(&work);
    assert(map.ready && map.first && !strcmp(map.first->identity, "/target/a/v2.0.0/src/renamed"));
    source_clone_map_destroy(&map); finish();
}
/** Exact tests suffixes and Unicode component bytes survive; invalid and failed
 * candidate construction leaves the output sentinel and input bytes unchanged. */
static void it_preserves_test_helper_suffixes_and_refuses_invalid_additional_candidates(void) {
    prepare(); SourceArena work; source_arena_init(&work, sources.allocator);
    ModuleSource helper = {.logical_path = "/vendor/pkg/v1.0.0/tests/дерево/helper"};
    SourceVersionAssignment assignment = {&helper, "/target/hierarchy/pkg/v2.0.0-speed"};
    const char *output = "sentinel";
    assert(source_additional_module_identity(&work, &assignment, &output) == SOURCE_OK);
    assert(!strcmp(output, "/target/hierarchy/pkg/v2.0.0-speed/tests/дерево/helper"));
    output = "sentinel"; size_t before = allocation_count;
    assert(source_additional_module_identity(NULL, &assignment, &output) == SOURCE_INVALID_ARGUMENT);
    assert(source_additional_module_identity(&work, NULL, &output) == SOURCE_INVALID_ARGUMENT);
    assert(source_additional_module_identity(&work, &assignment, NULL) == SOURCE_INVALID_ARGUMENT);
    assignment.original = NULL;
    assert(source_additional_module_identity(&work, &assignment, &output) == SOURCE_INVALID_ARGUMENT);
    assignment.original = &helper;
    const char *const bad_versions[] = {NULL, "//plain", "/target/pkg/v2.0.0/src/helper", "../v2.0.0", "/target/pkg/v2.0.0/"};
    for (size_t i = 0; i < sizeof(bad_versions) / sizeof(bad_versions[0]); i++) {
        assignment.version = bad_versions[i];
        assert(source_additional_module_identity(&work, &assignment, &output) == SOURCE_INVALID_ARGUMENT);
    }
    assignment.version = "/target/pkg/v2.0.0";
    const char *const bad_originals[] = {NULL, "//plain", "/vendor/pkg/v1.0.0", "/vendor/pkg/v1.0.0/src", "./helper"};
    for (size_t i = 0; i < sizeof(bad_originals) / sizeof(bad_originals[0]); i++) {
        helper.logical_path = bad_originals[i];
        assert(source_additional_module_identity(&work, &assignment, &output) == SOURCE_INVALID_ARGUMENT);
    }
    assert(allocation_count == before && !strcmp(output, "sentinel"));
    helper.logical_path = "/vendor/pkg/v1.0.0/tests/helper";
    failure_at = allocation_count + 1;
    assert(source_additional_module_identity(&work, &assignment, &output) == SOURCE_OUT_OF_MEMORY);
    assert(!strcmp(output, "sentinel")); failure_at = 0;
    source_arena_destroy(&work); finish();
}
