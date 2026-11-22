/** @file
 * @brief Enumerate exact unit-member suffixes independently and check storage/failure contracts.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/mapping/unit/member.h"
/** Main, flat companions and complete plus subtrees preserve exact suffixes;
 * prefix peers and companion-like subdirectories are not members. */
static void it_places_exact_unit_members_without_confusing_companions_or_prefix_peers(void) {
    static const struct { const char *member, *expected; bool directory; } cases[] = {
        {"/source/old name", "/target/new name", false},
        {"/source/old name.c", "/target/new name.c", false},
        {"/source/old name.native instructions.cmake", "/target/new name.native instructions.cmake", false},
        {"/source/old name+", "/target/new name+", true},
        {"/source/old name+/empty", "/target/new name+/empty", true},
        {"/source/old name+/child+/opaque.data", "/target/new name+/child+/opaque.data", false},
        {"/source/old names", NULL, false}, {"/source/old name.", NULL, false},
        {"/source/old name", NULL, true}, {"/source/old name.c", NULL, true},
        {"/source/old name.c/child", NULL, false}, {"/source/old name.c/child", NULL, true},
        {"/source/old name+", NULL, false}, {"/source/old name+extra", NULL, true},
        {"/elsewhere/old name", NULL, false}, {"/source/old", NULL, false}};
    setup_test(); SourceArena arena; source_arena_init(&arena, sources.allocator);
    for (size_t i = 0; i < sizeof(cases) / sizeof(*cases); i++) {
        const char *output = "unchanged"; size_t before = allocation_count;
        SourceUnitPathStatus status = source_unit_member_path(&arena, "/source/old name", "/target/new name",
            cases[i].member, cases[i].directory, &output);
        assert(status == (cases[i].expected ? SOURCE_UNIT_PATH_OK : SOURCE_UNIT_PATH_OUTSIDE));
        if (cases[i].expected) { assert(!strcmp(output, cases[i].expected)); }
        else { assert(!strcmp(output, "unchanged") && allocation_count == before); }
    }
    char destination[] = "/new", member[] = "/old.c"; const char *owned = NULL;
    assert(source_unit_member_path(&arena, "/old", destination, member, false, &owned) == SOURCE_UNIT_PATH_OK);
    destination[1] = 'X'; member[1] = 'Y'; assert(!strcmp(owned, "/new.c"));
    source_arena_destroy(&arena); destroy_test();
}
/** Invalid interfaces and the single allocation failure never publish a path;
 * complete real tree consumers separately exercise captured-unit composition. */
static void it_preserves_unit_member_output_at_invalid_interfaces_and_allocation_failure(void) {
    setup_test(); SourceArena arena; source_arena_init(&arena, sources.allocator);
    for (unsigned profile = 0; profile < 8; profile++) {
        const char *output = "unchanged"; size_t before = allocation_count;
        const char *original = profile == 1 ? NULL : profile == 2 ? "/" : "/old";
        const char *destination = profile == 3 ? "relative" : profile == 4 ? "/new/" : "/new";
        const char *member = profile == 5 ? NULL : profile == 6 ? "relative" : "/old.c";
        assert(source_unit_member_path(profile == 0 ? NULL : &arena, original, destination, member, false,
            profile == 7 ? NULL : &output) == SOURCE_UNIT_PATH_INVALID);
        assert(!strcmp(output, "unchanged") && allocation_count == before);
    }
    const char *output = "unchanged"; failure_at = allocation_count + 1;
    assert(source_unit_member_path(&arena, "/old", "/new", "/old.c", false, &output) == SOURCE_UNIT_PATH_MEMORY);
    assert(!strcmp(output, "unchanged")); failure_at = 0;
    source_arena_destroy(&arena); destroy_test();
}
