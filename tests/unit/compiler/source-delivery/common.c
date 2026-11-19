/** @file
 * @brief Own a source-only open-template library physically separate from its consuming Ama project.
 * @see [T191 contract](../../../../docs/implementation/template-source-delivery.md)
 */
#ifndef AMA_SOURCE_DELIVERY_TEST_COMMON
#define AMA_SOURCE_DELIVERY_TEST_COMMON
#include "../common.c"

/** Repeated uses denote one Type/code identity; private counters belong to each
 * ordinary instance. The independent branch is observable to input snapshots. */
static const char *const delivery_program =
    "@!\nquiet\n.independent\nfirst\n.lib.assembly\n ...result\nsecond\n.lib.assembly\n ...result\nsink\n.lib.sink\nfirst !\n < |17\n > result\nsink !\n < result\nsecond !\n < |17\n > result\nsink !\n < result\nfirst !\n < |23\n > result\nsink !\n < result\n";

/** One closed export and one deliberately partial public output are source
 * metadata; only the closed export is selected by ordinary consumers. */
static const char *const delivery_assembly_source =
    "@...\n...result\n.lib.forward\n ...slot < .lib.value\n...unused\n.lib.forward\n";

/** Write the existing concrete native companion protocol without a compiled or
 * serialized template interface. Compiled cases complete these test-owned bodies. */
static void delivery_native(const char *name, const char *interface) {
    char path[192];
    const char *suffixes[] = {"", ".h", ".c", ".abi"};
    const char *contents[] = {interface,
        "struct AMA_NATIVE(State) { size_t number; size_t calls; const void *address; };\n",
        "/* Authored fixture body, completed only by compiled consumer cases. */\n",
        "+ initialize\n- destroy\n| literal\n= clone\n? condition\n[] index\n"};
    for (size_t index = 0; index < sizeof(suffixes) / sizeof(suffixes[0]); index++) {
        int length = snprintf(path, sizeof(path), "generated/library/lib/%s%s", name, suffixes[index]);
        assert(length > 0 && (size_t)length < sizeof(path));
        file(path, contents[index]);
    }
}

/** The physical library root contributes explicit lib path components; its
 * directory name itself does not add an implicit logical namespace prefix. */
static void prepare_delivery_sources(void) {
    directory("generated/project");
    directory("generated/library");
    directory("generated/library/lib");
    file("generated/project/main", delivery_program);
    file("generated/project/independent", "@!\n");
    file("generated/library/lib/forward", "@<\ninput\n...slot\n@>\noutput\n...slot\n input\n@!\nprivate\n.lib.counter\nprivate !\noutput\n input\n");
    file("generated/library/lib/assembly", delivery_assembly_source);
    delivery_native("value", "@C\n");
    delivery_native("counter", "@C\n");
    delivery_native("sink", "@<\ninput\n.lib.value\n@>\noutput\n.lib.value\n@C\n");
}
#endif
